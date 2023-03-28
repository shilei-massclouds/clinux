// SPDX-License-Identifier: GPL-2.0-only

#include <bug.h>
#include <fork.h>
#include <slab.h>
#include <errno.h>
#include <fcntl.h>
#include <mount.h>
#include <namei.h>
#include <export.h>
#include <kernel.h>
#include <limits.h>
#include <signal.h>
#include <string.h>
#include <binfmts.h>
#include <current.h>
#include <highmem.h>
#include <uaccess.h>
#include <resource.h>
#include <processor.h>
#include <mmu_context.h>

static LIST_HEAD(formats);

void __register_binfmt(struct linux_binfmt * fmt, int insert)
{
    BUG_ON(!fmt);
    BUG_ON(!fmt->load_binary);
    insert ? list_add(&fmt->lh, &formats) :
        list_add_tail(&fmt->lh, &formats);
}
EXPORT_SYMBOL(__register_binfmt);

static int __bprm_mm_init(struct linux_binprm *bprm)
{
    int err;
    struct mm_struct *mm = bprm->mm;
    struct vm_area_struct *vma = NULL;

    bprm->vma = vma = vm_area_alloc(mm);
    if (!vma)
        return -ENOMEM;
    vma_set_anonymous(vma);

    vma->vm_end = STACK_TOP_MAX;
    vma->vm_start = vma->vm_end - PAGE_SIZE;
    vma->vm_flags = VM_STACK_FLAGS | VM_STACK_INCOMPLETE_SETUP;
    vma->vm_page_prot = vm_get_page_prot(vma->vm_flags);

    err = insert_vm_struct(mm, vma);
    if (err)
        panic("can not insert vma!");

    mm->stack_vm = mm->total_vm = 1;
    bprm->p = vma->vm_end - sizeof(void *);
    return 0;
}

/*
 * Create a new mm_struct and populate it with a temporary stack
 * vm_area_struct.  We don't have enough context at this point to
 * set the stack flags, permissions, and offset, so we use
 * temporary values.  We'll update them later in setup_arg_pages().
 */
static int bprm_mm_init(struct linux_binprm *bprm)
{
    int err;
    struct mm_struct *mm = NULL;

    bprm->mm = mm = mm_alloc();
    err = -ENOMEM;
    if (!mm)
        panic("no memory!");

    /* Save current stack limit for all calculations made during exec. */
    bprm->rlim_stack = current->signal->rlim[RLIMIT_STACK];

    err = __bprm_mm_init(bprm);
    if (err)
        panic("bad mm for bprm!");

    return 0;
}

static struct linux_binprm *alloc_bprm(int fd, struct filename *filename)
{
    struct linux_binprm *bprm = kzalloc(sizeof(*bprm), GFP_KERNEL);
    int retval = -ENOMEM;
    if (!bprm)
        return ERR_PTR(retval);

    if (fd == AT_FDCWD || filename->name[0] == '/') {
        bprm->filename = filename->name;
    } else {
        if (filename->name[0] == '\0')
            bprm->fdpath = kasprintf(GFP_KERNEL, "/dev/fd/%d", fd);
        else
            bprm->fdpath = kasprintf(GFP_KERNEL, "/dev/fd/%d/%s",
                                     fd, filename->name);
        if (!bprm->fdpath)
            return ERR_PTR(retval);

        bprm->filename = bprm->fdpath;
    }
    bprm->interp = bprm->filename;

    printk("%s: (%s)!\n", __func__, filename->name);

    retval = bprm_mm_init(bprm);
    if (retval)
        panic("can not init bprm mm!");
    return bprm;
}

static int count_strings_kernel(const char *const *argv)
{
    int i;

    if (!argv)
        return 0;

    for (i = 0; argv[i]; ++i) {
        if (i >= MAX_ARG_STRINGS)
            return -E2BIG;
    }
    return i;
}

static int bprm_stack_limits(struct linux_binprm *bprm)
{
    unsigned long limit, ptr_size;

    /*
     * Limit to 1/4 of the max stack size or 3/4 of _STK_LIM
     * (whichever is smaller) for the argv+env strings.
     * This ensures that:
     *  - the remaining binfmt code will not run out of stack space,
     *  - the program will have a reasonable amount of stack left
     *    to work from.
     */
    limit = _STK_LIM / 4 * 3;
    limit = min(limit, bprm->rlim_stack.rlim_cur / 4);
    /*
     * We've historically supported up to 32 pages (ARG_MAX)
     * of argument strings even with small stacks
     */
    limit = max_t(unsigned long, limit, ARG_MAX);
    /*
     * We must account for the size of all the argv and envp pointers to
     * the argv and envp strings, since they will also take up space in
     * the stack. They aren't stored until much later when we can't
     * signal to the parent that the child has run out of stack space.
     * Instead, calculate it here so it's possible to fail gracefully.
     */
    ptr_size = (bprm->argc + bprm->envc) * sizeof(void *);
    if (limit <= ptr_size)
        return -E2BIG;
    limit -= ptr_size;

    bprm->argmin = bprm->p - limit;
    return 0;
}

static bool valid_arg_len(struct linux_binprm *bprm, long len)
{
    return len <= MAX_ARG_STRLEN;
}

/*
 * The nascent bprm->mm is not visible until exec_mmap() but it can
 * use a lot of memory, account these pages in current->mm temporary
 * for oom_badness()->get_mm_rss(). Once exec succeeds or fails, we
 * change the counter back via acct_arg_size(0).
 */
static void acct_arg_size(struct linux_binprm *bprm, unsigned long pages)
{
    struct mm_struct *mm = current->mm;
    long diff = (long)(pages - bprm->vma_pages);

    if (!mm || !diff)
        return;

    bprm->vma_pages = pages;
}

static struct page *
get_arg_page(struct linux_binprm *bprm, unsigned long pos, int write)
{
    int ret;
    struct page *page;
    unsigned int gup_flags = FOLL_FORCE;

    if (write)
        gup_flags |= FOLL_WRITE;

    /*
     * We are doing an exec().  'current' is the process
     * doing the exec and bprm->mm is the new process's mm.
     */
    ret = get_user_pages_remote(bprm->mm, pos, 1, gup_flags, &page,
                                NULL, NULL);
    if (ret <= 0)
        return NULL;

    if (write)
        acct_arg_size(bprm, vma_pages(bprm->vma));

    return page;
}

/*
 * Copy and argument/environment string from the kernel to the processes stack.
 */
int copy_string_kernel(const char *arg, struct linux_binprm *bprm)
{
    unsigned long pos = bprm->p;
    int len = strnlen(arg, MAX_ARG_STRLEN) + 1 /* terminating NUL */;

    if (len == 0)
        return -EFAULT;
    if (!valid_arg_len(bprm, len))
        return -E2BIG;

    /* We're going to work our way backwards. */
    arg += len;
    bprm->p -= len;
    if (bprm->p < bprm->argmin)
        return -E2BIG;

    while (len > 0) {
        char *kaddr;
        struct page *page;
        unsigned int bytes_to_copy =
            min_t(unsigned int, len,
                  min_not_zero(offset_in_page(pos), PAGE_SIZE));

        pos -= bytes_to_copy;
        arg -= bytes_to_copy;
        len -= bytes_to_copy;

        page = get_arg_page(bprm, pos, 1);
        if (!page)
            panic("out of memory!");
        kaddr = kmap_atomic(page);
        memcpy(kaddr + offset_in_page(pos), arg, bytes_to_copy);
    }

    return 0;
}

static int
copy_strings_kernel(int argc, const char *const *argv,
                    struct linux_binprm *bprm)
{
    while (argc-- > 0) {
        int ret = copy_string_kernel(argv[argc], bprm);
        if (ret < 0)
            return ret;
    }
    return 0;
}

static struct file *
do_open_execat(int fd, struct filename *name, int flags)
{
    int err;
    struct file *file;
    struct open_flags open_exec_flags = {
        .open_flag = O_LARGEFILE | O_RDONLY | __FMODE_EXEC,
        .acc_mode = MAY_EXEC,
        .intent = LOOKUP_OPEN,
        .lookup_flags = LOOKUP_FOLLOW,
    };

    file = do_filp_open(fd, name, &open_exec_flags);
    if (IS_ERR(file))
        panic("bad file!");

    return file;
}

static int prepare_binprm(struct linux_binprm *bprm)
{
    loff_t pos = 0;

    memset(bprm->buf, 0, BINPRM_BUF_SIZE);
    return kernel_read(bprm->file, bprm->buf, BINPRM_BUF_SIZE, &pos);
}

static int search_binary_handler(struct linux_binprm *bprm)
{
    int retval;
    struct linux_binfmt *fmt;

    retval = prepare_binprm(bprm);
    if (retval < 0)
        panic("prepare binprm error!");

    list_for_each_entry(fmt, &formats, lh) {
        retval = fmt->load_binary(bprm);
        if (bprm->point_of_no_return || (retval != -ENOEXEC))
            return retval;

        panic("bad fmt");
    }

    panic("%s: !", __func__);
}

static int exec_binprm(struct linux_binprm *bprm)
{
    int ret, depth;

    for (depth = 0;; depth++) {
        struct file *exec;
        if (depth > 5)
            return -ELOOP;

        ret = search_binary_handler(bprm);
        if (ret < 0)
            panic("can not find handler!");
        if (!bprm->interpreter)
            break;

        panic("interpreter exists!");
    }

    return 0;
}

/*
 * sys_execve() executes a new program.
 */
static int bprm_execve(struct linux_binprm *bprm,
                       int fd, struct filename *filename, int flags)
{
    int retval;
    struct file *file;

    file = do_open_execat(fd, filename, flags);
    retval = PTR_ERR(file);
    if (IS_ERR(file))
        panic("bad file!");

    bprm->file = file;

    retval = exec_binprm(bprm);
    if (retval < 0)
        panic("exec binprm error!");

    /* execve succeeded */
    return retval;
}

int kernel_execve(const char *kernel_filename,
                  const char *const *argv, const char *const *envp)
{
    int retval;
    struct filename *filename;
    struct linux_binprm *bprm;
    int fd = AT_FDCWD;

    filename = getname_kernel(kernel_filename);
    if (IS_ERR(filename))
        panic("bad filename!");

    bprm = alloc_bprm(fd, filename);
    if (IS_ERR(bprm))
        panic("can not alloc bprm!");

    retval = count_strings_kernel(argv);
    if (retval < 0)
        panic("out of memory!");
    bprm->argc = retval;

    retval = count_strings_kernel(envp);
    if (retval < 0)
        panic("out of memory!");
    bprm->envc = retval;

    retval = bprm_stack_limits(bprm);
    if (retval < 0)
        panic("out of limits!");

    retval = copy_string_kernel(bprm->filename, bprm);
    if (retval < 0)
        panic("out of memory!");
    bprm->exec = bprm->p;

    retval = copy_strings_kernel(bprm->envc, envp, bprm);
    if (retval < 0)
        panic("out of memory!");

    retval = copy_strings_kernel(bprm->argc, argv, bprm);
    if (retval < 0)
        panic("out of memory!");

    retval = bprm_execve(bprm, fd, filename, 0);
    return retval;
}
EXPORT_SYMBOL(kernel_execve);

static int exec_mmap(struct mm_struct *mm)
{
    struct task_struct *tsk;
    struct mm_struct *old_mm, *active_mm;

    tsk = current;
    old_mm = current->mm;

    active_mm = tsk->active_mm;
    tsk->mm = mm;
    tsk->active_mm = mm;
    activate_mm(active_mm, mm);
    if (old_mm) {
        BUG_ON(active_mm != old_mm);
        return 0;
    }
    return 0;
}

/*
 * Calling this is the point of no return. None of the failures will be
 * seen by userspace since either the process is already taking a fatal
 * signal (via de_thread() or coredump), or will have SEGV raised
 * (after exec_mmap()) by search_binary_handler (see below).
 */
int begin_new_exec(struct linux_binprm *bprm)
{
    int retval;

    /*
     * Ensure all future errors are fatal.
     */
    bprm->point_of_no_return = true;

    /*
     * Must be called _before_ exec_mmap() as bprm->mm is
     * not visibile until then. This also enables the update
     * to be lockless.
     */
    set_mm_exe_file(bprm->mm, bprm->file);

    retval = exec_mmap(bprm->mm);
    if (retval)
        panic("exec mmap error!");

    bprm->mm = NULL;

    /*
     * Ensure that the uaccess routines can actually operate on userspace
     * pointers:
     */
    force_uaccess_begin();

    return 0;
}

void setup_new_exec(struct linux_binprm *bprm)
{
    /* Setup things that can depend upon the personality */
    struct task_struct *me = current;

    arch_pick_mmap_layout(me->mm, &bprm->rlim_stack);

    /* Set the new mm task size. We have to do that late because it may
     * depend on TIF_32BIT which is only updated in flush_thread() on
     * some architectures like powerpc
     */
    me->mm->task_size = TASK_SIZE;
}
EXPORT_SYMBOL(setup_new_exec);

static int shift_arg_pages(struct vm_area_struct *vma, unsigned long shift)
{
    panic("%s: !", __func__);
    /* Todo: add it. */
    return 0;
}

int setup_arg_pages(struct linux_binprm *bprm,
                    unsigned long stack_top,
                    int executable_stack)
{
    unsigned long ret;
    unsigned long vm_flags;
    unsigned long stack_base;
    unsigned long stack_size;
    unsigned long stack_expand;
    unsigned long rlim_stack;
    struct mm_struct *mm = current->mm;
    struct vm_area_struct *vma = bprm->vma;

    stack_top = PAGE_ALIGN(stack_top);

    mm->arg_start = bprm->p;

    printk("%s: ### p(%lx)\n", __func__, bprm->p);

    vm_flags = VM_STACK_FLAGS;

    /*
     * Adjust stack execute permissions; explicitly enable for
     * EXSTACK_ENABLE_X, disable for EXSTACK_DISABLE_X and leave alone
     * (arch default) otherwise.
     */
    if (unlikely(executable_stack == EXSTACK_ENABLE_X))
        vm_flags |= VM_EXEC;
    else if (executable_stack == EXSTACK_DISABLE_X)
        vm_flags &= ~VM_EXEC;
    vm_flags |= mm->def_flags;
    vm_flags |= VM_STACK_INCOMPLETE_SETUP;

    if (unlikely(vm_flags & VM_EXEC)) {
        panic("process '%lx' started with executable stack", bprm->file);
    }

    /* mprotect_fixup is overkill to remove the temporary stack flags */
    vma->vm_flags &= ~VM_STACK_INCOMPLETE_SETUP;

    stack_expand = 131072UL; /* randomly 32*4k (or 2*64k) pages */
    stack_size = vma->vm_end - vma->vm_start;

    /*
     * Align this down to a page boundary as expand_stack
     * will align it up.
     */
    rlim_stack = bprm->rlim_stack.rlim_cur & PAGE_MASK;

    if (stack_size + stack_expand > rlim_stack)
        stack_base = vma->vm_end - rlim_stack;
    else
        stack_base = vma->vm_start - stack_expand;

    current->mm->start_stack = bprm->p;

    printk("%s: ### p(%lx) stack_base(%lx) rlim_stack(%lx)\n",
           __func__, bprm->p, stack_base, rlim_stack);

    ret = expand_stack(vma, stack_base);
    if (ret)
        ret = -EFAULT;

    return ret;
}
EXPORT_SYMBOL(setup_arg_pages);

void set_binfmt(struct linux_binfmt *new)
{
    struct mm_struct *mm = current->mm;
    mm->binfmt = new;
}
EXPORT_SYMBOL(set_binfmt);

/* Runs immediately before start_thread() takes over. */
void finalize_exec(struct linux_binprm *bprm)
{
    /* Store any stack rlimit changes before starting thread. */
    current->signal->rlim[RLIMIT_STACK] = bprm->rlim_stack;
}
EXPORT_SYMBOL(finalize_exec);

struct file *open_exec(const char *name)
{
    struct filename *filename = getname_kernel(name);
    struct file *f = ERR_CAST(filename);

    if (!IS_ERR(filename))
        f = do_open_execat(AT_FDCWD, filename, 0);
    return f;
}
EXPORT_SYMBOL(open_exec);
