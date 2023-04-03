// SPDX-License-Identifier: GPL-2.0-only

#include <mm.h>
#include <slab.h>
#include <export.h>
#include <current.h>
#include <uaccess.h>
#include <mm_types.h>
#include <processor.h>
#include <mman-common.h>

void __vma_link_list(struct mm_struct *mm, struct vm_area_struct *vma,
                     struct vm_area_struct *prev)
{
    struct vm_area_struct *next;

    vma->vm_prev = prev;
    if (prev) {
        next = prev->vm_next;
        prev->vm_next = vma;
    } else {
        next = mm->mmap;
        mm->mmap = vma;
    }
    vma->vm_next = next;
    if (next)
        next->vm_prev = vma;
}
EXPORT_SYMBOL(__vma_link_list);

unsigned long
vm_mmap_pgoff(struct file *file, unsigned long addr,
              unsigned long len, unsigned long prot,
              unsigned long flag, unsigned long pgoff)
{
    unsigned long ret;
    unsigned long populate;
    LIST_HEAD(uf);

    ret = do_mmap(file, addr, len, prot, flag, pgoff, &populate, &uf);
    if (populate)
        mm_populate(ret, populate);

    return ret;
}

unsigned long vm_mmap(struct file *file, unsigned long addr,
                      unsigned long len, unsigned long prot,
                      unsigned long flag, unsigned long offset)
{
    if (unlikely(offset + PAGE_ALIGN(len) < offset))
        panic("bad arg!");
    if (unlikely(offset_in_page(offset)))
        panic("bad arg!");

    return vm_mmap_pgoff(file, addr, len, prot, flag, offset >> PAGE_SHIFT);
}
EXPORT_SYMBOL(vm_mmap);

/*
 * Leave enough space between the mmap area and the stack to honour ulimit in
 * the face of randomisation.
 */
#define MIN_GAP     (SZ_128M)
#define MAX_GAP     (STACK_TOP / 6 * 5)

static unsigned long
mmap_base(unsigned long rnd, struct rlimit *rlim_stack)
{
    unsigned long gap = rlim_stack->rlim_cur;
    unsigned long pad = stack_guard_gap;

    /* Values close to RLIM_INFINITY can overflow. */
    if (gap + pad > gap)
        gap += pad;

    if (gap < MIN_GAP)
        gap = MIN_GAP;
    else if (gap > MAX_GAP)
        gap = MAX_GAP;

    return PAGE_ALIGN(STACK_TOP - gap - rnd);
}

#define arch_get_mmap_end(addr) (TASK_SIZE)

/*
 * Same as find_vma, but also return a pointer to the previous VMA in *pprev.
 */
/*
struct vm_area_struct *
find_vma_prev(struct mm_struct *mm, unsigned long addr,
              struct vm_area_struct **pprev)
{
    struct vm_area_struct *vma;

    vma = find_vma(mm, addr);
    if (vma) {
        *pprev = vma->vm_prev;
    } else {
        struct rb_node *rb_node = rb_last(&mm->mm_rb);

        *pprev = rb_node ? rb_entry(rb_node, struct vm_area_struct, vm_rb) : NULL;
    }
    return vma;
}
*/

void
arch_pick_mmap_layout(struct mm_struct *mm, struct rlimit *rlim_stack)
{
    unsigned long random_factor = 0UL;

    mm->mmap_base = mmap_base(random_factor, rlim_stack);
    mm->get_unmapped_area = arch_get_unmapped_area_topdown;
}
EXPORT_SYMBOL(arch_pick_mmap_layout);

/**
 * memdup_user - duplicate memory region from user space
 *
 * @src: source address in user space
 * @len: number of bytes to copy
 *
 * Return: an ERR_PTR() on failure.  Result is physically
 * contiguous, to be freed by kfree().
 */
void *memdup_user(const void *src, size_t len)
{
    void *p;

    p = kmalloc(len, GFP_USER | __GFP_NOWARN);
    if (!p)
        return ERR_PTR(-ENOMEM);

    if (copy_from_user(p, src, len)) {
        kfree(p);
        return ERR_PTR(-EFAULT);
    }
    return p;
}
EXPORT_SYMBOL(memdup_user);

/**
 * strndup_user - duplicate an existing string from user space
 * @s: The string to duplicate
 * @n: Maximum number of bytes to copy, including the trailing NUL.
 *
 * Return: newly allocated copy of @s or an ERR_PTR() in case of error
 */
char *strndup_user(const char *s, long n)
{
    char *p;
    long length;

    length = strnlen_user(s, n);

    if (!length)
        return ERR_PTR(-EFAULT);

    if (length > n)
        return ERR_PTR(-EINVAL);

    p = memdup_user(s, length);
    if (IS_ERR(p))
        return p;

    p[length - 1] = '\0';
    return p;
}
EXPORT_SYMBOL(strndup_user);
