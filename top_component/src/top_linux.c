// SPDX-License-Identifier: GPL-2.0-only

#ifndef DEBUG
#define DEBUG       /* Enable initcall_debug */
#endif

#include <linux/types.h>
#include <linux/export.h>
#include <linux/sched/task.h>
#include <linux/smp.h>
#include <linux/cgroup.h>
#include <linux/cpu.h>
#include <linux/security.h>
#include <linux/of_fdt.h>
#include <linux/dma-direct.h>
#include <linux/initrd.h>
#include <linux/extable.h>
#include <linux/kmemleak.h>
#include <linux/pti.h>
#include <linux/ftrace.h>
#include <linux/sched/init.h>
#include <linux/sched/isolation.h>
#include <linux/context_tracking.h>
#include <linux/random.h>
#include <linux/stackprotector.h>
#include <linux/perf_event.h>
#include <linux/profile.h>
#include <asm/pgtable.h>
#include <asm/sbi.h>
#include <cl_hook.h>
#include <cl_types.h>
#include "../../booter/src/booter.h"

/*
 * Boot command-line arguments
 */
#define MAX_INIT_ARGS CONFIG_INIT_ENV_ARG_LIMIT
#define MAX_INIT_ENVS CONFIG_INIT_ENV_ARG_LIMIT

/* Untouched saved command line (eg. for /proc) */
char *saved_command_line;

/* Command line for parameter parsing */
static char *static_command_line;
/* Untouched extra command line */
static char *extra_command_line;
/* Extra init arguments */
static char *extra_init_args;

#ifdef CONFIG_BOOT_CONFIG
/* Is bootconfig on command line? */
static bool bootconfig_found;
static bool initargs_found;
#else
# define bootconfig_found false
# define initargs_found false
#endif

bool initcall_debug;
core_param(initcall_debug, initcall_debug, bool, 0644);

extern void setup_vm_final(void);
extern void free_area_init(unsigned long *max_zone_pfn);
extern void init_IRQ(void);
extern void time_init(void);

static void setup_zero_page(void)
{
    memset((void *)empty_zero_page, 0, PAGE_SIZE);
}

static void __init zone_sizes_init(void)
{
    unsigned long max_zone_pfns[MAX_NR_ZONES] = { 0, };

#ifdef CONFIG_ZONE_DMA32
    max_zone_pfns[ZONE_DMA32] = PFN_DOWN(min(4UL * SZ_1G,
            (unsigned long) PFN_PHYS(max_low_pfn)));
#endif
    max_zone_pfns[ZONE_NORMAL] = max_low_pfn;

    free_area_init(max_zone_pfns);
}

static void __init resource_init(void)
{
    struct memblock_region *region;

    for_each_memblock(memory, region) {
        struct resource *res;

        res = memblock_alloc(sizeof(struct resource), SMP_CACHE_BYTES);
        if (!res)
            panic("%s: Failed to allocate %zu bytes\n", __func__,
                  sizeof(struct resource));

        if (memblock_is_nomap(region)) {
            res->name = "reserved";
            res->flags = IORESOURCE_MEM;
        } else {
            res->name = "System RAM";
            res->flags = IORESOURCE_SYSTEM_RAM | IORESOURCE_BUSY;
        }
        res->start = __pfn_to_phys(memblock_region_memory_base_pfn(region));
        res->end = __pfn_to_phys(memblock_region_memory_end_pfn(region)) - 1;

        request_resource(&iomem_resource, res);
    }
}

void __init paging_init(void)
{
    setup_vm_final();
    sparse_init();
    setup_zero_page();
    zone_sizes_init();
    resource_init();
}

void __init setup_arch(char **cmdline_p)
{
    setup_kernel_in_mm();
    *cmdline_p = boot_command_line;

    parse_early_param();

    setup_bootmem();
    paging_init();
#if IS_ENABLED(CONFIG_BUILTIN_DTB)
    unflatten_and_copy_device_tree();
#else
    unflatten_device_tree();
#endif

#ifdef CONFIG_SWIOTLB
    swiotlb_init(1);
#endif

#ifdef CONFIG_KASAN
    kasan_init();
#endif

#if IS_ENABLED(CONFIG_RISCV_SBI)
    sbi_init();
#endif

#ifdef CONFIG_SMP
    setup_smp();
#endif

    riscv_fill_hwcap();
}

#ifdef CONFIG_BLK_DEV_INITRD
static void * __init get_boot_config_from_initrd(u32 *_size, u32 *_csum)
{
    u32 size, csum;
    char *data;
    u32 *hdr;

    if (!initrd_end)
        return NULL;

    panic("Cannot reach here!\n");
}
#else
static void * __init get_boot_config_from_initrd(u32 *_size, u32 *_csum)
{
    return NULL;
}
#endif

#ifdef CONFIG_BOOT_CONFIG
#error bad config 'CONFIG_BOOT_CONFIG'
#else
static void __init setup_boot_config(const char *cmdline)
{
    /* Remove bootconfig data from initrd */
    get_boot_config_from_initrd(NULL, NULL);
}
#endif

/*
 * We need to store the untouched command line for future reference.
 * We also need to store the touched command line since the parameter
 * parsing is performed in place, and we should allow a component to
 * store reference of name/value for future reference.
 */
static void __init setup_command_line(char *command_line)
{
	size_t len, xlen = 0, ilen = 0;

	if (extra_command_line)
		xlen = strlen(extra_command_line);
	if (extra_init_args)
		ilen = strlen(extra_init_args) + 4; /* for " -- " */

	len = xlen + strlen(boot_command_line) + 1;

	saved_command_line = memblock_alloc(len + ilen, SMP_CACHE_BYTES);
	if (!saved_command_line)
		panic("%s: Failed to allocate %zu bytes\n", __func__, len + ilen);

	static_command_line = memblock_alloc(len, SMP_CACHE_BYTES);
	if (!static_command_line)
		panic("%s: Failed to allocate %zu bytes\n", __func__, len);

	if (xlen) {
		/*
		 * We have to put extra_command_line before boot command
		 * lines because there could be dashes (separator of init
		 * command line) in the command lines.
		 */
		strcpy(saved_command_line, extra_command_line);
		strcpy(static_command_line, extra_command_line);
	}
	strcpy(saved_command_line + xlen, boot_command_line);
	strcpy(static_command_line + xlen, command_line);

	if (ilen) {
		/*
		 * Append supplemental init boot args to saved_command_line
		 * so that user can check what command line options passed
		 * to init.
		 */
		len = strlen(saved_command_line);
		if (initargs_found) {
			saved_command_line[len++] = ' ';
		} else {
			strcpy(saved_command_line + len, " -- ");
			len += 4;
		}

		strcpy(saved_command_line + len, extra_init_args);
	}
}

#ifndef CONFIG_SMP
static const unsigned int setup_max_cpus = NR_CPUS;
static inline void setup_nr_cpu_ids(void) { }
static inline void smp_prepare_cpus(unsigned int maxcpus) { }
#endif

static const char *argv_init[MAX_INIT_ARGS+2] = { "init", NULL, };
const char *envp_init[MAX_INIT_ENVS+2] = { "HOME=/", "TERM=linux", NULL, };
static const char *panic_later, *panic_param;

extern const struct obs_kernel_param __setup_start[], __setup_end[];

static bool __init obsolete_checksetup(char *line)
{
    const struct obs_kernel_param *p;
    bool had_early_param = false;

    p = __setup_start;
    do {
        int n = strlen(p->str);
        if (parameqn(line, p->str, n)) {
            if (p->early) {
                /* Already done in parse_early_param?
                 * (Needs exact match on param part).
                 * Keep iterating, as we can have early
                 * params and __setups of same names 8( */
                if (line[n] == '\0' || line[n] == '=')
                    had_early_param = true;
            } else if (!p->setup_func) {
                pr_warn("Parameter %s is obsolete, ignored\n",
                    p->str);
                return true;
            } else if (p->setup_func(line + n))
                return true;
        }
        p++;
    } while (p < __setup_end);

    return had_early_param;
}

/* Change NUL term back to "=", to make "param" the whole string. */
static void __init repair_env_string(char *param, char *val)
{
    if (val) {
        /* param=val or param="val"? */
        if (val == param+strlen(param)+1)
            val[-1] = '=';
        else if (val == param+strlen(param)+2) {
            val[-2] = '=';
            memmove(val-1, val, strlen(val)+1);
        } else
            BUG();
    }
}

/*
 * Unknown boot options get handed to init, unless they look like
 * unused parameters (modprobe will find them in /proc/cmdline).
 */
static int __init unknown_bootoption(char *param, char *val,
				     const char *unused, void *arg)
{
	size_t len = strlen(param);

	repair_env_string(param, val);

	/* Handle obsolete-style parameters */
	if (obsolete_checksetup(param))
		return 0;

	/* Unused module parameter. */
	if (strnchr(param, len, '.'))
		return 0;

	if (panic_later)
		return 0;

	if (val) {
		/* Environment option */
		unsigned int i;
		for (i = 0; envp_init[i]; i++) {
			if (i == MAX_INIT_ENVS) {
				panic_later = "env";
				panic_param = param;
			}
			if (!strncmp(param, envp_init[i], len+1))
				break;
		}
		envp_init[i] = param;
	} else {
		/* Command line option */
		unsigned int i;
		for (i = 0; argv_init[i]; i++) {
			if (i == MAX_INIT_ARGS) {
				panic_later = "init";
				panic_param = param;
			}
		}
		argv_init[i] = param;
	}
	return 0;
}

/* Anything after -- gets handed straight to init. */
static int __init set_init_arg(char *param, char *val,
                   const char *unused, void *arg)
{
    unsigned int i;

    if (panic_later)
        return 0;

    repair_env_string(param, val);

    for (i = 0; argv_init[i]; i++) {
        if (i == MAX_INIT_ARGS) {
            panic_later = "init";
            panic_param = param;
            return 0;
        }
    }
    argv_init[i] = param;
    return 0;
}

/* Report memory auto-initialization states for this boot. */
static void __init report_meminit(void)
{
    const char *stack;

    if (IS_ENABLED(CONFIG_INIT_STACK_ALL_PATTERN))
        stack = "all(pattern)";
    else if (IS_ENABLED(CONFIG_INIT_STACK_ALL_ZERO))
        stack = "all(zero)";
    else if (IS_ENABLED(CONFIG_GCC_PLUGIN_STRUCTLEAK_BYREF_ALL))
        stack = "byref_all(zero)";
    else if (IS_ENABLED(CONFIG_GCC_PLUGIN_STRUCTLEAK_BYREF))
        stack = "byref(zero)";
    else if (IS_ENABLED(CONFIG_GCC_PLUGIN_STRUCTLEAK_USER))
        stack = "__user(zero)";
    else
        stack = "off";

    pr_info("mem auto-init: stack:%s, heap alloc:%s, heap free:%s\n",
        stack, want_init_on_alloc(GFP_KERNEL) ? "on" : "off",
        want_init_on_free() ? "on" : "off");
    if (want_init_on_free())
        pr_info("mem auto-init: clearing system memory may take some time...\n");
}

#if defined(CONFIG_MMU) && defined(CONFIG_DEBUG_VM)
static inline void print_mlk(char *name, unsigned long b, unsigned long t)
{
    pr_notice("%12s : 0x%08lx - 0x%08lx   (%4ld kB)\n", name, b, t,
          (((t) - (b)) >> 10));
}

static inline void print_mlm(char *name, unsigned long b, unsigned long t)
{
    pr_notice("%12s : 0x%08lx - 0x%08lx   (%4ld MB)\n", name, b, t,
          (((t) - (b)) >> 20));
}

static void print_vm_layout(void)
{
    pr_notice("Virtual kernel memory layout:\n");
    print_mlk("fixmap", (unsigned long)FIXADDR_START,
          (unsigned long)FIXADDR_TOP);
    print_mlm("pci io", (unsigned long)PCI_IO_START,
          (unsigned long)PCI_IO_END);
    print_mlm("vmemmap", (unsigned long)VMEMMAP_START,
          (unsigned long)VMEMMAP_END);
    print_mlm("vmalloc", (unsigned long)VMALLOC_START,
          (unsigned long)VMALLOC_END);
    print_mlm("lowmem", (unsigned long)PAGE_OFFSET,
          (unsigned long)high_memory);
}
#else
static void print_vm_layout(void) { }
#endif

/* From 'arch/riscv/mm/init.c'. */
void __init mem_init(void)
{
#ifdef CONFIG_FLATMEM
    BUG_ON(!mem_map);
#endif /* CONFIG_FLATMEM */

    high_memory = (void *)(__va(PFN_PHYS(max_low_pfn)));
    memblock_free_all();

    mem_init_print_info(NULL);
    print_vm_layout();
}

void __init __weak pgtable_cache_init(void) { }

/*
 * Set up kernel memory allocators
 */
static void __init mm_init(void)
{
    /*
     * page_ext requires contiguous pages,
     * bigger than MAX_ORDER unless SPARSEMEM.
     */
    page_ext_init_flatmem();
    init_debug_pagealloc();
    report_meminit();
    mem_init();
    kmem_cache_init();
    kmemleak_init();
    pgtable_init();
    debug_objects_mem_init();
    vmalloc_init();
    ioremap_huge_init();
    /* Should be run before the first non-init thread is created */
    init_espfix_bsp();
    /* Should be run after espfix64 is set up. */
    pti_init();
}

#ifdef TRACEPOINTS_ENABLED
static void __init initcall_debug_enable(void);
#else
static inline void initcall_debug_enable(void)
{
}
#endif

int
cl_top_linux_init(void)
{
    char *command_line;
    char *after_dashes;

    sbi_puts("module[top_linux]: init begin ...\n");
    REQUIRE_COMPONENT(early_printk);
    REQUIRE_COMPONENT(of_irq);
    REQUIRE_COMPONENT(irqchip);
    REQUIRE_COMPONENT(timer_riscv);

    //
    // start_kernel (init/main.c)
    //

    REQUIRE_COMPONENT(task); // Setup 'tp => init_task' here from head.S.
    parse_dtb();             // Move 'parse_dtb' here from head.S.

    set_task_stack_end_magic(&init_task);
    smp_setup_processor_id();
    debug_objects_early_init();

    cgroup_init_early();

    local_irq_disable();
    early_boot_irqs_disabled = true;

    /*
     * Interrupts are still disabled. Do necessary setups, then
     * enable them.
     */
    boot_cpu_init();
    page_address_init();
    pr_notice("%s", linux_banner);
    early_security_init();
    setup_arch(&command_line);
    setup_boot_config(command_line);
    setup_command_line(command_line);
    setup_nr_cpu_ids();
    setup_per_cpu_areas();
    smp_prepare_boot_cpu(); /* arch-specific boot-cpu hooks */
    boot_cpu_hotplug_init();

    build_all_zonelists(NULL);
    page_alloc_init();

    pr_notice("Kernel command line: %s\n", saved_command_line);
    /* parameters may set static keys */
    jump_label_init();

    // Note: some early params may not be setup.
    parse_early_param();
    after_dashes = parse_args("Booting kernel",
                  static_command_line, __start___param,
                  __stop___param - __start___param,
                  -1, -1, NULL, &unknown_bootoption);
    if (!IS_ERR_OR_NULL(after_dashes))
        parse_args("Setting init args", after_dashes, NULL, 0, -1, -1,
               NULL, set_init_arg);
    if (extra_init_args)
        parse_args("Setting extra init args", extra_init_args,
               NULL, 0, -1, -1, NULL, set_init_arg);

    /*
     * These use large bootmem allocations and must precede
     * kmem_cache_init()
     */
    setup_log_buf(0);
    vfs_caches_init_early();
    sort_main_extable();
    trap_init();
    mm_init();

    ftrace_init();

    /* trace_printk can be enabled here */
    early_trace_init();

    /*
     * Set up the scheduler prior starting any interrupts (such as the
     * timer interrupt). Full topology setup happens at smp_init()
     * time - but meanwhile we still have a functioning scheduler.
     */
    sched_init();

    /*
     * Disable preemption - early bootup scheduling is extremely
     * fragile until we cpu_idle() for the first time.
     */
    preempt_disable();
    if (WARN(!irqs_disabled(),
         "Interrupts were enabled *very* early, fixing it\n"))
        local_irq_disable();
    radix_tree_init();

    /*
     * Set up housekeeping before setting up workqueues to allow the unbound
     * workqueue to take non-housekeeping into account.
     */
    housekeeping_init();

    /*
     * Allow workqueue creation and work item queueing/cancelling
     * early.  Work item execution depends on kthreads and starts after
     * workqueue_init().
     */
    workqueue_init_early();

    rcu_init();

    /* Trace events are available after this */
    trace_init();

    if (initcall_debug)
        initcall_debug_enable();

    context_tracking_init();
    /* init some links before init_ISA_irqs() */
    early_irq_init();
    init_IRQ();
    tick_init();
    rcu_init_nohz();
    init_timers();
    hrtimers_init();
    softirq_init();
    timekeeping_init();

    /*
     * For best initial stack canary entropy, prepare it after:
     * - setup_arch() for any UEFI RNG entropy and boot cmdline access
     * - timekeeping_init() for ktime entropy used in rand_initialize()
     * - rand_initialize() to get any arch-specific entropy like RDRAND
     * - add_latent_entropy() to get any latent entropy
     * - adding command line entropy
     */
    rand_initialize();
    add_latent_entropy();
    add_device_randomness(command_line, strlen(command_line));
    boot_init_stack_canary();

    time_init();
    perf_event_init();
    profile_init();
    call_function_init();
    WARN(!irqs_disabled(), "Interrupts were enabled early\n");

    early_boot_irqs_disabled = false;
    local_irq_enable();

    kmem_cache_init_late();

    sbi_puts("module[top_linux]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_top_linux_init);
