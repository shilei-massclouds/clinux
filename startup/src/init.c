// SPDX-License-Identifier: GPL-2.0-only

#include <bug.h>
#include <sbi.h>
#include <uts.h>
#include <fork.h>
#include <slab.h>
#include <sched.h>
#include <export.h>
#include <kernel.h>
#include <ptrace.h>
#include <signal.h>
#include <fdtable.h>
#include <filemap.h>
#include <nsproxy.h>
#include <utsname.h>

void (*handle_arch_irq)(struct pt_regs *);
EXPORT_SYMBOL(handle_arch_irq);

/* to be mentioned only in INIT_TASK */
struct fs_struct init_fs = {
};

static struct signal_struct init_signals = {
    .rlim   = INIT_RLIMITS,
};

/*
 * Default task group.
 * Every task in system belongs to this group at bootup.
 */
struct task_group root_task_group;
EXPORT_SYMBOL(root_task_group);

struct uts_namespace init_uts_ns = {
    .name = {
        .sysname    = UTS_SYSNAME,
        .nodename   = UTS_NODENAME,
        .release    = UTS_RELEASE,
        .version    = UTS_VERSION,
        .machine    = UTS_MACHINE,
        .domainname = UTS_DOMAINNAME,
    },
};

struct nsproxy init_nsproxy = {
    .uts_ns = &init_uts_ns,
};

struct files_struct init_files = {
    .fdt        = &init_files.fdtab,
    .fdtab      = {
        .max_fds        = NR_OPEN_DEFAULT,
        .fd             = &init_files.fd_array[0],
        .close_on_exec  = init_files.close_on_exec_init,
        .open_fds   = init_files.open_fds_init,
        .full_fds_bits  = init_files.full_fds_bits_init,
    },
};

struct task_struct init_task
__aligned(L1_CACHE_BYTES) = {
    .thread_info = INIT_THREAD_INFO(init_task),

    .stack  = init_stack,
    .flags  = PF_KTHREAD,
    .fs     = &init_fs,
    .files  = &init_files,
    .signal = &init_signals,
    .nsproxy = &init_nsproxy,

    .normal_prio = MAX_PRIO - 20,
    .sched_task_group = &root_task_group,
};

gfp_t GFP_KERNEL_C = GFP_KERNEL;
EXPORT_SYMBOL(GFP_KERNEL_C);

gfp_t __GFP_ZERO_C = __GFP_ZERO;
EXPORT_SYMBOL(__GFP_ZERO_C);

/*
 * Slab
 */

/* For common cache allocation based on size */
kmalloc_t kmalloc;
EXPORT_SYMBOL(kmalloc);

krealloc_t krealloc;
EXPORT_SYMBOL(krealloc);

kfree_t kfree;
EXPORT_SYMBOL(kfree);

kmemdup_nul_t kmemdup_nul;
EXPORT_SYMBOL(kmemdup_nul);

/* For fork */
vm_area_alloc_t vm_area_alloc;
EXPORT_SYMBOL(vm_area_alloc);

vm_area_dup_t vm_area_dup;
EXPORT_SYMBOL(vm_area_dup);

/* For specific cache allocation */
kmem_cache_alloc_t kmem_cache_alloc;
EXPORT_SYMBOL(kmem_cache_alloc);

kmem_cache_free_t kmem_cache_free;
EXPORT_SYMBOL(kmem_cache_free);

/* For block device */
struct super_block *blockdev_superblock;
EXPORT_SYMBOL(blockdev_superblock);

I_BDEV_T I_BDEV;
EXPORT_SYMBOL(I_BDEV);

/* For filemap */
add_to_page_cache_lru_t add_to_page_cache_lru;
EXPORT_SYMBOL(add_to_page_cache_lru);

/* For sched */
schedule_tail_t schedule_tail_func;
EXPORT_SYMBOL(schedule_tail_func);

extern struct task_struct *
__switch_to(struct task_struct *, struct task_struct *);
EXPORT_SYMBOL(__switch_to);

extern void ret_from_kernel_thread(void);
EXPORT_SYMBOL(ret_from_kernel_thread);

void schedule_tail(struct task_struct *p)
{
    schedule_tail_func(p);
}

void unreachable(void)
{
    sbi_puts("\n##########################");
    sbi_puts("\nImpossible to come here!\n");
    sbi_puts("##########################\n");

    halt();
}

extern char _end[];
EXPORT_SYMBOL(_end);

extern char ekernel[];
EXPORT_SYMBOL(ekernel);

extern char _text_start[];
EXPORT_SYMBOL(_text_start);

extern char _text_end[];
EXPORT_SYMBOL(_text_end);

extern char _rodata_start[];
EXPORT_SYMBOL(_rodata_start);

extern char _rodata_end[];
EXPORT_SYMBOL(_rodata_end);

extern char _data_start[];
EXPORT_SYMBOL(_data_start);

extern char _data_end[];
EXPORT_SYMBOL(_data_end);

extern char _bss_start[];
EXPORT_SYMBOL(_bss_start);

extern char _bss_stop[];
EXPORT_SYMBOL(_bss_stop);

extern char _init_stack[];
EXPORT_SYMBOL(_init_stack);

extern char _init_stack_top[];
EXPORT_SYMBOL(_init_stack_top);
