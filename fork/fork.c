// SPDX-License-Identifier: GPL-2.0-only

#include <bug.h>
#include <pid.h>
#include <fork.h>
#include <slab.h>
#include <task.h>
#include <errno.h>
#include <sched.h>
#include <export.h>
#include <printk.h>
#include <string.h>
#include <current.h>
#include <cpumask.h>
#include <pgalloc.h>
#include <mm_types.h>
#include <user_namespace.h>
#include <syscalls.h>

#define allocate_mm()   (kmem_cache_alloc(mm_cachep, GFP_KERNEL))

/* SLAB cache for vm_area_struct structures */
static struct kmem_cache *vm_area_cachep;

/* SLAB cache for mm_struct structures (tsk->mm) */
static struct kmem_cache *mm_cachep;

static inline int mm_alloc_pgd(struct mm_struct *mm)
{
    mm->pgd = pgd_alloc(mm);
    if (unlikely(!mm->pgd))
        return -ENOMEM;
    return 0;
}

static struct mm_struct *
mm_init(struct mm_struct *mm, struct task_struct *p)
{
    if (mm_alloc_pgd(mm))
        panic("bad memory!");

    return mm;
}

/*
 * Allocate and initialize an mm_struct.
 */
struct mm_struct *mm_alloc(void)
{
    struct mm_struct *mm;

    mm = allocate_mm();
    if (!mm)
        return NULL;

    memset(mm, 0, sizeof(*mm));
    return mm_init(mm, current);
}
EXPORT_SYMBOL(mm_alloc);

struct vm_area_struct *_vm_area_alloc(struct mm_struct *mm)
{
    struct vm_area_struct *vma;

    vma = kmem_cache_alloc(vm_area_cachep, GFP_KERNEL);
    if (vma)
        vma_init(vma, mm);
    return vma;
}

void set_mm_exe_file(struct mm_struct *mm, struct file *new_exe_file)
{
    mm->exe_file = new_exe_file;
}
EXPORT_SYMBOL(set_mm_exe_file);

static struct kmem_cache *task_struct_cachep;

static inline struct task_struct *alloc_task_struct_node(void)
{
    return kmem_cache_alloc(task_struct_cachep, GFP_KERNEL);
}

static unsigned long *alloc_thread_stack_node(struct task_struct *tsk)
{
    struct page *page = alloc_pages(THREADINFO_GFP, THREAD_SIZE_ORDER);

    if (likely(page)) {
        tsk->stack = page_address(page);
        return tsk->stack;
    }
    return NULL;
}

static struct task_struct *
dup_task_struct(struct task_struct *orig)
{
    int err;
    unsigned long *stack;
    struct task_struct *tsk;

    tsk = alloc_task_struct_node();
    if (!tsk)
        panic("out of memory!");

    stack = alloc_thread_stack_node(tsk);
    if (!stack)
        panic("out of memory!");

    err = arch_dup_task_struct(tsk, orig);

    /*
     * arch_dup_task_struct() clobbers the stack-related fields.  Make
     * sure they're properly initialized before using any stack-related
     * functions again.
     */
    tsk->stack = stack;
    return tsk;
}

static struct task_struct *
copy_process(struct pid *pid, struct kernel_clone_args *args)
{
    int retval;
    struct task_struct *p;
    u64 clone_flags = args->flags;

    p = dup_task_struct(current);
    if (!p)
        panic("dup task struct error!");

    /* Perform scheduler related setup. Assign this task to a CPU. */
    retval = sched_fork(clone_flags, p);
    if (retval)
        panic("bad fork!");

    retval = copy_thread(clone_flags, args->stack, args->stack_size, p, args->tls);
    if (retval)
        panic("bad fork cleanup io!");

    return p;
}

/*
 *  Ok, this is the main fork-routine.
 *
 * It copies the process, and if successful kick-starts
 * it and waits for it to finish using the VM if required.
 *
 * args->exit_signal is expected to be checked for sanity by the caller.
 */
long _do_fork(struct kernel_clone_args *args)
{
    struct task_struct *p;

    p = copy_process(NULL, args);
    if (IS_ERR(p))
        panic("copy process error(%d)", PTR_ERR(p));

    wake_up_new_task(p);
    return 0;
}

/*
 * Create a kernel thread.
 */
pid_t kernel_thread(int (*fn)(void *), void *arg, unsigned long flags)
{
    struct kernel_clone_args args = {
        .flags      = ((lower_32_bits(flags) | CLONE_VM | CLONE_UNTRACED)
                       & ~CSIGNAL),
        .stack      = (unsigned long)fn,
        .stack_size = (unsigned long)arg,
    };

    return _do_fork(&args);
}
EXPORT_SYMBOL(kernel_thread);

#define ARCH_MIN_MMSTRUCT_ALIGN 0

void proc_caches_init(void)
{
    unsigned int mm_size;

    /*
     * The mm_cpumask is located at the end of mm_struct, and is
     * dynamically sized based on the maximum CPU number this system
     * can have, taking hotplug into account (nr_cpu_ids).
     */
    mm_size = sizeof(struct mm_struct) + cpumask_size();

    mm_cachep =
        kmem_cache_create_usercopy("mm_struct", mm_size,
                                   ARCH_MIN_MMSTRUCT_ALIGN,
                                   SLAB_HWCACHE_ALIGN|SLAB_PANIC|SLAB_ACCOUNT,
                                   offsetof(struct mm_struct, saved_auxv),
                                   sizeof_field(struct mm_struct, saved_auxv),
                                   NULL);

    vm_area_cachep = KMEM_CACHE(vm_area_struct, SLAB_PANIC|SLAB_ACCOUNT);
}

static void
task_struct_whitelist(unsigned long *offset, unsigned long *size)
{
    /* Fetch thread_struct whitelist for the architecture. */
    arch_thread_struct_whitelist(offset, size);

    /*
     * Handle zero-sized whitelist or empty thread_struct, otherwise
     * adjust offset to position of thread_struct in task_struct.
     */
    if (unlikely(*size == 0))
        *offset = 0;
    else
        *offset += offsetof(struct task_struct, thread);
}

struct vm_area_struct *_vm_area_dup(struct vm_area_struct *orig)
{
    struct vm_area_struct *new = kmem_cache_alloc(vm_area_cachep, GFP_KERNEL);

    if (new) {
        /*
         * orig->shared.rb may be modified concurrently, but the clone
         * will be reinitialized.
         */
        *new = *orig;
        //INIT_LIST_HEAD(&new->anon_vma_chain);
        new->vm_next = new->vm_prev = NULL;
    }
    return new;
}

#define ARCH_MIN_TASKALIGN  0

void fork_init(void)
{
    unsigned long useroffset, usersize;
    int align = max_t(int, L1_CACHE_BYTES, ARCH_MIN_TASKALIGN);

    /* create a slab on which task_structs can be allocated */
    task_struct_whitelist(&useroffset, &usersize);

    task_struct_cachep =
        kmem_cache_create_usercopy("task_struct",
                                   arch_task_struct_size, align,
                                   SLAB_PANIC|SLAB_ACCOUNT,
                                   useroffset, usersize, NULL);
}

static long _do_set_tid_address(int *tidptr)
{
    return 0;
    //current->clear_child_tid = tidptr;

    //return task_pid_vnr(current);
}

static int
init_module(void)
{
    printk("module[fork]: init begin ...\n");

    fork_init();

    proc_caches_init();

    vm_area_alloc = _vm_area_alloc;
    vm_area_dup = _vm_area_dup;
    do_set_tid_address = _do_set_tid_address;

    printk("module[fork]: init end!\n");

    return 0;
}
