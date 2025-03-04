// SPDX-License-Identifier: GPL-2.0-only

#ifndef DEBUG
#define DEBUG       /* Enable initcall_debug */
#endif

#include <linux/types.h>
#include <linux/export.h>
#include <linux/sched/task.h>
#include <linux/smp.h>
#include <linux/padata.h>
#include <linux/nmi.h>
#include <linux/sfi.h>
#include <linux/cpuset.h>
#include <linux/taskstats_kern.h>
#include <linux/delayacct.h>
#include <linux/cgroup.h>
#include <linux/cpu.h>
#include <linux/security.h>
#include <linux/of_fdt.h>
#include <linux/proc_ns.h>
#include <linux/dma-direct.h>
#include <linux/proc_fs.h>
#include <linux/initrd.h>
#include <linux/extable.h>
#include <linux/kmemleak.h>
#include <linux/pti.h>
#include <linux/ftrace.h>
#include <linux/binfmts.h>
#include <linux/sched/init.h>
#include <linux/sched/isolation.h>
#include <linux/sched/clock.h>
#include <linux/context_tracking.h>
#include <linux/kprobes.h>
#include <linux/random.h>
#include <linux/stackprotector.h>
#include <linux/perf_event.h>
#include <linux/profile.h>
#include <linux/async.h>
#include <linux/console.h>
#include <linux/mempolicy.h>
#include <linux/acpi.h>
#include <linux/delay.h>
#include <linux/rmap.h>
#include <linux/utsname.h>
#include <linux/buffer_head.h>
#include <linux/kgdb.h>
#include <linux/init_syscalls.h>
#include <linux/integrity.h>
#include <linux/rodata_test.h>
#include <asm/bugs.h>
#include <asm/pgtable.h>
#include <asm/sbi.h>

#define CREATE_TRACE_POINTS
#include <trace/events/initcall.h>

#include <cl_hook.h>
#include <cl_types.h>
#include "../../booter/src/booter.h"

/*
 * Boot command-line arguments
 */
#define MAX_INIT_ARGS CONFIG_INIT_ENV_ARG_LIMIT
#define MAX_INIT_ENVS CONFIG_INIT_ENV_ARG_LIMIT

extern initcall_entry_t __initcall_start[];
extern initcall_entry_t __initcall0_start[];
extern initcall_entry_t __initcall1_start[];
extern initcall_entry_t __initcall2_start[];
extern initcall_entry_t __initcall3_start[];
extern initcall_entry_t __initcall4_start[];
extern initcall_entry_t __initcall5_start[];
extern initcall_entry_t __initcall6_start[];
extern initcall_entry_t __initcall7_start[];
extern initcall_entry_t __initcall_end[];

static initcall_entry_t *initcall_levels[] __initdata = {
    __initcall0_start,
    __initcall1_start,
    __initcall2_start,
    __initcall3_start,
    __initcall4_start,
    __initcall5_start,
    __initcall6_start,
    __initcall7_start,
    __initcall_end,
};

#if defined(CONFIG_STRICT_KERNEL_RWX) || defined(CONFIG_STRICT_MODULE_RWX)
bool rodata_enabled __ro_after_init = true;
static int __init set_debug_rodata(char *str)
{
    return strtobool(str, &rodata_enabled);
}
__setup("rodata=", set_debug_rodata);
#endif

/* Untouched saved command line (eg. for /proc) */
extern char *saved_command_line;

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

//bool initcall_debug;
//core_param(initcall_debug, initcall_debug, bool, 0644);

static char *execute_command;
static char *ramdisk_execute_command = "/init";

extern void setup_vm_final(void);
extern void free_area_init(unsigned long *max_zone_pfn);
extern void init_IRQ(void);
extern void time_init(void);
/* Default late time init is NULL. archs can override this later. */
void (*__initdata late_time_init)(void);

void __init __weak mem_encrypt_init(void) { }

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

# if THREAD_SIZE >= PAGE_SIZE
void __init __weak thread_stack_cache_init(void)
{
}
#endif

void __init __weak poking_init(void) { }

void __init __weak arch_post_acpi_subsys_init(void) { }

static noinline void __init kernel_init_freeable(void);

#ifdef CONFIG_KALLSYMS
struct blacklist_entry {
    struct list_head next;
    char *buf;
};

static __initdata_or_module LIST_HEAD(blacklisted_initcalls);

static int __init initcall_blacklist(char *str)
{
    char *str_entry;
    struct blacklist_entry *entry;

    /* str argument is a comma-separated list of functions */
    do {
        str_entry = strsep(&str, ",");
        if (str_entry) {
            pr_debug("blacklisting initcall %s\n", str_entry);
            entry = memblock_alloc(sizeof(*entry),
                           SMP_CACHE_BYTES);
            if (!entry)
                panic("%s: Failed to allocate %zu bytes\n",
                      __func__, sizeof(*entry));
            entry->buf = memblock_alloc(strlen(str_entry) + 1,
                            SMP_CACHE_BYTES);
            if (!entry->buf)
                panic("%s: Failed to allocate %zu bytes\n",
                      __func__, strlen(str_entry) + 1);
            strcpy(entry->buf, str_entry);
            list_add(&entry->next, &blacklisted_initcalls);
        }
    } while (str_entry);

    return 0;
}

static bool __init_or_module initcall_blacklisted(initcall_t fn)
{
    struct blacklist_entry *entry;
    char fn_name[KSYM_SYMBOL_LEN];
    unsigned long addr;

    if (list_empty(&blacklisted_initcalls))
        return false;

    addr = (unsigned long) dereference_function_descriptor(fn);
    sprint_symbol_no_offset(fn_name, addr);

    /*
     * fn will be "function_name [module_name]" where [module_name] is not
     * displayed for built-in init functions.  Strip off the [module_name].
     */
    strreplace(fn_name, ' ', '\0');

    list_for_each_entry(entry, &blacklisted_initcalls, next) {
        if (!strcmp(fn_name, entry->buf)) {
            pr_debug("initcall %s blacklisted\n", fn_name);
            return true;
        }
    }

    return false;
}
#endif

static __init_or_module void
trace_initcall_start_cb(void *data, initcall_t fn)
{
    ktime_t *calltime = (ktime_t *)data;

    printk(KERN_DEBUG "calling  %pS @ %i\n", fn, task_pid_nr(current));
    *calltime = ktime_get();
}

static __init_or_module void
trace_initcall_finish_cb(void *data, initcall_t fn, int ret)
{
    ktime_t *calltime = (ktime_t *)data;
    ktime_t delta, rettime;
    unsigned long long duration;

    rettime = ktime_get();
    delta = ktime_sub(rettime, *calltime);
    duration = (unsigned long long) ktime_to_ns(delta) >> 10;
    printk(KERN_DEBUG "initcall %pS returned %d after %lld usecs\n",
         fn, ret, duration);
}

static ktime_t initcall_calltime;

#ifdef TRACEPOINTS_ENABLED
static void __init initcall_debug_enable(void)
{
    int ret;

    ret = register_trace_initcall_start(trace_initcall_start_cb,
                        &initcall_calltime);
    ret |= register_trace_initcall_finish(trace_initcall_finish_cb,
                          &initcall_calltime);
    WARN(ret, "Failed to register initcall tracepoints\n");
}
# define do_trace_initcall_start    trace_initcall_start
# define do_trace_initcall_finish   trace_initcall_finish
#else
static inline void do_trace_initcall_start(initcall_t fn)
{
    if (!initcall_debug)
        return;
    trace_initcall_start_cb(&initcall_calltime, fn);
}
static inline void do_trace_initcall_finish(initcall_t fn, int ret)
{
    if (!initcall_debug)
        return;
    trace_initcall_finish_cb(&initcall_calltime, fn, ret);
}
#endif /* !TRACEPOINTS_ENABLED */

int __init_or_module do_one_initcall(initcall_t fn)
{
    int count = preempt_count();
    char msgbuf[64];
    int ret;

    if (initcall_blacklisted(fn))
        return -EPERM;

    printk("=================>\n");
    do_trace_initcall_start(fn);
    ret = fn();
    do_trace_initcall_finish(fn, ret);
    printk("<=================\n");

    msgbuf[0] = 0;

    if (preempt_count() != count) {
        sprintf(msgbuf, "preemption imbalance ");
        preempt_count_set(count);
    }
    if (irqs_disabled()) {
        strlcat(msgbuf, "disabled interrupts ", sizeof(msgbuf));
        local_irq_enable();
    }
    WARN(msgbuf[0], "initcall %pS returned with %s\n", fn, msgbuf);

    add_latent_entropy();
    return ret;
}

#ifdef CONFIG_STRICT_KERNEL_RWX
static void mark_readonly(void)
{
    if (rodata_enabled) {
        /*
         * load_module() results in W+X mappings, which are cleaned
         * up with call_rcu().  Let's make sure that queued work is
         * flushed so that we don't hit false positives looking for
         * insecure pages which are W+X.
         */
        rcu_barrier();
        mark_rodata_ro();
        rodata_test();
    } else
        pr_info("Kernel memory protection disabled.\n");
}
#elif defined(CONFIG_ARCH_HAS_STRICT_KERNEL_RWX)
static inline void mark_readonly(void)
{
    pr_warn("Kernel memory protection not selected by kernel config.\n");
}
#else
static inline void mark_readonly(void)
{
    pr_warn("This architecture does not have kernel memory protection.\n");
}
#endif

void __weak free_initmem(void)
{
    free_initmem_default(POISON_FREE_INITMEM);
}

static int run_init_process(const char *init_filename)
{
    const char *const *p;

    argv_init[0] = init_filename;
    pr_info("Run %s as init process\n", init_filename);
    pr_debug("  with arguments:\n");
    for (p = argv_init; *p; p++)
        pr_debug("    %s\n", *p);
    pr_debug("  with environment:\n");
    for (p = envp_init; *p; p++)
        pr_debug("    %s\n", *p);
    return kernel_execve(init_filename, argv_init, envp_init);
}

static int try_to_run_init_process(const char *init_filename)
{
    int ret;

    ret = run_init_process(init_filename);

    if (ret && ret != -ENOENT) {
        pr_err("Starting init: %s exists but couldn't execute it (error %d)\n",
               init_filename, ret);
    }

    return ret;
}

static int __init init_setup(char *str)
{
    unsigned int i;

    execute_command = str;
    /*
     * In case LILO is going to boot us with default command line,
     * it prepends "auto" before the whole cmdline which makes
     * the shell think it should execute a script with such name.
     * So we ignore all arguments entered _before_ init=... [MJ]
     */
    for (i = 1; i < MAX_INIT_ARGS; i++)
        argv_init[i] = NULL;
    return 1;
}
__setup("init=", init_setup);

static int __ref kernel_init(void *unused)
{
    int ret;

    kernel_init_freeable();
    /* need to finish all async __init code before freeing the memory */
    async_synchronize_full();
    kprobe_free_init_mem();
    ftrace_free_init_mem();
    free_initmem();
    mark_readonly();

    /*
     * Kernel mappings are now finalized - update the userspace page-table
     * to finalize PTI.
     */
    pti_finalize();

    system_state = SYSTEM_RUNNING;
    numa_default_policy();

    rcu_end_inkernel_boot();

    do_sysctl_args();

    if (ramdisk_execute_command) {
        ret = run_init_process(ramdisk_execute_command);
        if (!ret)
            return 0;
        pr_err("Failed to execute %s (error %d)\n",
               ramdisk_execute_command, ret);
    }

    /*
     * We try each of these until one succeeds.
     *
     * The Bourne shell can be used instead of init if we are
     * trying to recover a really broken machine.
     */
    if (execute_command) {
        ret = run_init_process(execute_command);
        if (!ret)
            return 0;
        panic("Requested init %s failed (error %d).",
              execute_command, ret);
    }

    if (CONFIG_DEFAULT_INIT[0] != '\0') {
        ret = run_init_process(CONFIG_DEFAULT_INIT);
        if (ret)
            pr_err("Default init %s failed (error %d)\n",
                   CONFIG_DEFAULT_INIT, ret);
        else
            return 0;
    }

    if (!try_to_run_init_process("/sbin/init") ||
        !try_to_run_init_process("/etc/init") ||
        !try_to_run_init_process("/bin/init") ||
        !try_to_run_init_process("/bin/sh"))
        return 0;

    panic("No working init found.  Try passing init= option to kernel. "
          "See Linux Documentation/admin-guide/init.rst for guidance.");
}

/*
 * We need to finalize in a non-__init function or else race conditions
 * between the root thread and the init thread may cause start_kernel to
 * be reaped by free_initmem before the root thread has proceeded to
 * cpu_idle.
 *
 * gcc-3.4 accidentally inlines this function, so use noinline.
 */

static __initdata DECLARE_COMPLETION(kthreadd_done);

noinline void __ref rest_init(void)
{
    struct task_struct *tsk;
    int pid;

    rcu_scheduler_starting();
    /*
     * We need to spawn init first so that it obtains pid 1, however
     * the init task will end up wanting to create kthreads, which, if
     * we schedule it before we create kthreadd, will OOPS.
     */
    pid = kernel_thread(kernel_init, NULL, CLONE_FS);
    /*
     * Pin init on the boot CPU. Task migration is not properly working
     * until sched_init_smp() has been run. It will set the allowed
     * CPUs for init to the non isolated CPUs.
     */
    rcu_read_lock();
    tsk = find_task_by_pid_ns(pid, &init_pid_ns);
    set_cpus_allowed_ptr(tsk, cpumask_of(smp_processor_id()));
    rcu_read_unlock();

    numa_default_policy();
    pid = kernel_thread(kthreadd, NULL, CLONE_FS | CLONE_FILES);
    rcu_read_lock();
    kthreadd_task = find_task_by_pid_ns(pid, &init_pid_ns);
    rcu_read_unlock();

    /*
     * Enable might_sleep() and smp_processor_id() checks.
     * They cannot be enabled earlier because with CONFIG_PREEMPTION=y
     * kernel_thread() would trigger might_sleep() splats. With
     * CONFIG_PREEMPT_VOLUNTARY=y the init task might have scheduled
     * already, but it's stuck on the kthreadd_done completion.
     */
    system_state = SYSTEM_SCHEDULING;

    complete(&kthreadd_done);

    /*
     * The boot idle thread must execute schedule()
     * at least once to get things moving:
     */
    schedule_preempt_disabled();
    /* Call into cpu_idle with preempt disabled */
    cpu_startup_entry(CPUHP_ONLINE);
    sbi_puts("module[top_linux]: rest_init ok!\n");
}

void __init __weak arch_call_rest_init(void)
{
    rest_init();
}

/* Keep these in sync with initcalls in include/linux/init.h */
static const char *initcall_level_names[] __initdata = {
    "pure",
    "core",
    "postcore",
    "arch",
    "subsys",
    "fs",
    "device",
    "late",
};

static int __init ignore_unknown_bootoption(char *param, char *val,
                   const char *unused, void *arg)
{
    return 0;
}

static void __init do_pre_smp_initcalls(void)
{
    initcall_entry_t *fn;

    trace_initcall_level("early");
    for (fn = __initcall_start; fn < __initcall0_start; fn++)
        do_one_initcall(initcall_from_entry(fn));
}

/* Call all constructor functions linked into the kernel. */
static void __init do_ctors(void)
{
#ifdef CONFIG_CONSTRUCTORS
    ctor_fn_t *fn = (ctor_fn_t *) __ctors_start;

    for (; fn < (ctor_fn_t *) __ctors_end; fn++)
        (*fn)();
#endif
}

static void __init do_initcall_level(int level, char *command_line)
{
    initcall_entry_t *fn;

    sbi_puts(initcall_level_names[level]);
    sbi_puts("\n");
    parse_args(initcall_level_names[level],
           command_line, __start___param,
           __stop___param - __start___param,
           level, level,
           NULL, ignore_unknown_bootoption);

    trace_initcall_level(initcall_level_names[level]);
    for (fn = initcall_levels[level]; fn < initcall_levels[level+1]; fn++)
        do_one_initcall(initcall_from_entry(fn));
}

static void __init do_initcalls(void)
{
    int level;
    size_t len = strlen(saved_command_line) + 1;
    char *command_line;

    command_line = kzalloc(len, GFP_KERNEL);
    if (!command_line)
        panic("%s: Failed to allocate %zu bytes\n", __func__, len);

    for (level = 0; level < ARRAY_SIZE(initcall_levels) - 1; level++) {
        /* Parser modifies command_line, restore it each time */
        strcpy(command_line, saved_command_line);
        do_initcall_level(level, command_line);
    }

    kfree(command_line);
    printk("%s: ========== END!\n", __func__);
}

/*
 * Ok, the machine is now initialized. None of the devices
 * have been touched yet, but the CPU subsystem is up and
 * running, and memory and process management works.
 *
 * Now we can finally start doing some real work..
 */
static void __init do_basic_setup(void)
{
    cpuset_init_smp();
    driver_init();
    init_irq_proc();
    do_ctors();
    usermodehelper_enable();
    do_initcalls();
}

/* Open /dev/console, for stdin/stdout/stderr, this should never fail */
void __init console_on_rootfs(void)
{
    struct file *file = filp_open("/dev/console", O_RDWR, 0);

    if (IS_ERR(file)) {
        pr_err("Warning: unable to open an initial console.\n");
        return;
    }
    init_dup(file);
    init_dup(file);
    init_dup(file);
    fput(file);
}

static noinline void __init kernel_init_freeable(void)
{
    /*
     * Wait until kthreadd is all set-up.
     */
    wait_for_completion(&kthreadd_done);

    /* Now the scheduler is fully set up and can do blocking allocations */
    gfp_allowed_mask = __GFP_BITS_MASK;

    /*
     * init can allocate pages on any node
     */
    set_mems_allowed(node_states[N_MEMORY]);

    cad_pid = task_pid(current);

    smp_prepare_cpus(setup_max_cpus);

    workqueue_init();

    init_mm_internals();

    do_pre_smp_initcalls();
    lockup_detector_init();

    smp_init();
    sched_init_smp();

    padata_init();
    page_alloc_init_late();
    /* Initialize page ext after all struct pages are initialized. */
    page_ext_init();

    do_basic_setup();

    console_on_rootfs();

    /*
     * check if there is an early userspace init.  If yes, let it do all
     * the work
     */
    if (init_eaccess(ramdisk_execute_command) != 0) {
        ramdisk_execute_command = NULL;
        prepare_namespace();
    }

    /*
     * Ok, we have completed the initial bootup, and
     * we're essentially up and running. Get rid of the
     * initmem segments and start the user-mode stuff..
     *
     * rootfs is available now, try loading the public keys
     * and default modules
     */

    integrity_load_keys();
}

int
cl_top_linux_init(void)
{
    char *command_line;
    char *after_dashes;

    sbi_puts("module[top_linux]: init begin ...\n");
    REQUIRE_COMPONENT(locks);
    REQUIRE_COMPONENT(of_irq);
    REQUIRE_COMPONENT(irqchip);
    REQUIRE_COMPONENT(timer_riscv);
    REQUIRE_COMPONENT(earlycon);
    REQUIRE_COMPONENT(show_mem);
    REQUIRE_COMPONENT(shmem);
    REQUIRE_COMPONENT(vmscan);
    REQUIRE_COMPONENT(fs_namespace);
    REQUIRE_COMPONENT(proc);
    REQUIRE_COMPONENT(namei);
    REQUIRE_COMPONENT(kthread);
    REQUIRE_COMPONENT(exit);
    REQUIRE_COMPONENT(exec);
    REQUIRE_COMPONENT(module);
    REQUIRE_COMPONENT(devtmpfs);
    REQUIRE_COMPONENT(security);
    REQUIRE_COMPONENT(pid_namespace);
    REQUIRE_COMPONENT(net_core);
    REQUIRE_COMPONENT(ksysfs);
    REQUIRE_COMPONENT(debugfs);
    REQUIRE_COMPONENT(riscv_fault);
    REQUIRE_COMPONENT(mmap);
    REQUIRE_COMPONENT(drv_char);
    REQUIRE_COMPONENT(notify);
    REQUIRE_COMPONENT(groups);
    REQUIRE_COMPONENT(8250);
    REQUIRE_COMPONENT(virtio_mmio);
    REQUIRE_COMPONENT(drv_clk);
    REQUIRE_COMPONENT(fs_writeback);
    REQUIRE_COMPONENT(noinitramfs);
    REQUIRE_COMPONENT(block_dev);
    REQUIRE_COMPONENT(virtio_blk);
    REQUIRE_COMPONENT(partitions);
    REQUIRE_COMPONENT(truncate);
    REQUIRE_COMPONENT(elevator);
    REQUIRE_COMPONENT(workingset);
    REQUIRE_COMPONENT(ext2);
    REQUIRE_COMPONENT(binfmt_elf);
    REQUIRE_COMPONENT(xattr);
    REQUIRE_COMPONENT(net_namespace);
    REQUIRE_COMPONENT(ipc_namespace);
    REQUIRE_COMPONENT(ipv4);
    REQUIRE_COMPONENT(tcp_ipv4);
    REQUIRE_COMPONENT(udp);
    REQUIRE_COMPONENT(ping);
    REQUIRE_COMPONENT(arp);

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

    /*
     * HACK ALERT! This is early. We're enabling the console before
     * we've done PCI setups etc, and console_init() must be aware of
     * this. But we do want output early, in case something goes wrong.
     */
    console_init();
    if (panic_later)
        panic("Too many boot %s vars at `%s'", panic_later,
              panic_param);

    lockdep_init();

    /*
     * Need to run this when irqs are enabled, because it wants
     * to self-test [hard/soft]-irqs on/off lock inversion bugs
     * too:
     */
    locking_selftest();

    /*
     * This needs to be called before any devices perform DMA
     * operations that might use the SWIOTLB bounce buffers. It will
     * mark the bounce buffers as decrypted so that their usage will
     * not cause "plain-text" data to be decrypted when accessed.
     */
    mem_encrypt_init();

#ifdef CONFIG_BLK_DEV_INITRD
    if (initrd_start && !initrd_below_start_ok &&
        page_to_pfn(virt_to_page((void *)initrd_start)) < min_low_pfn) {
        pr_crit("initrd overwritten (0x%08lx < 0x%08lx) - disabling it.\n",
            page_to_pfn(virt_to_page((void *)initrd_start)),
            min_low_pfn);
        initrd_start = 0;
    }
#endif
    setup_per_cpu_pageset();
    numa_policy_init();
    acpi_early_init();
    if (late_time_init)
        late_time_init();
    sched_clock_init();
    calibrate_delay();
    pid_idr_init();
    anon_vma_init();
#ifdef CONFIG_X86
    if (efi_enabled(EFI_RUNTIME_SERVICES))
        efi_enter_virtual_mode();
#endif
    thread_stack_cache_init();
    cred_init();
    fork_init();
    proc_caches_init();
    uts_ns_init();
    buffer_init();
    key_init();
    security_init();
    dbg_late_init();
    vfs_caches_init();
    pagecache_init();
    signals_init();
    seq_file_init();
    proc_root_init();
    nsfs_init();
    cpuset_init();
    cgroup_init();
    taskstats_init_early();
    delayacct_init();

    poking_init();
    check_bugs();

    acpi_subsystem_init();
    arch_post_acpi_subsys_init();
    sfi_init_late();
    kcsan_init();

    /* Do the rest non-__init'ed, we're now alive */
    arch_call_rest_init();

    prevent_tail_call_optimization();
    return 0;
}
EXPORT_SYMBOL(cl_top_linux_init);
