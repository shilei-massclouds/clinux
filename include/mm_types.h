/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_MM_TYPES_H
#define _LINUX_MM_TYPES_H

#include <mm.h>
#include <page.h>
#include <rbtree.h>

/* NEW_AUX_ENT entries in auxiliary table */
#define AT_VECTOR_SIZE_BASE 20  /* from "include/linux/auxvec.h" */
#define AT_VECTOR_SIZE (2 * (AT_VECTOR_SIZE_BASE + 1))

#define page_private(page)  ((page)->private)

/**
 * typedef vm_fault_t - Return type for page fault handlers.
 *
 * Page fault handlers return a bitmask of %VM_FAULT values.
 */
typedef unsigned int vm_fault_t;

typedef unsigned long vm_flags_t;

enum vm_fault_reason {
    VM_FAULT_OOM            = (__force vm_fault_t)0x000001,
    VM_FAULT_SIGBUS         = (__force vm_fault_t)0x000002,
    VM_FAULT_MAJOR          = (__force vm_fault_t)0x000004,
    VM_FAULT_WRITE          = (__force vm_fault_t)0x000008,
    VM_FAULT_HWPOISON       = (__force vm_fault_t)0x000010,
    VM_FAULT_HWPOISON_LARGE = (__force vm_fault_t)0x000020,
    VM_FAULT_SIGSEGV        = (__force vm_fault_t)0x000040,
    VM_FAULT_NOPAGE         = (__force vm_fault_t)0x000100,
    VM_FAULT_LOCKED         = (__force vm_fault_t)0x000200,
    VM_FAULT_RETRY          = (__force vm_fault_t)0x000400,
    VM_FAULT_FALLBACK       = (__force vm_fault_t)0x000800,
    VM_FAULT_DONE_COW       = (__force vm_fault_t)0x001000,
    VM_FAULT_NEEDDSYNC      = (__force vm_fault_t)0x002000,
    VM_FAULT_HINDEX_MASK    = (__force vm_fault_t)0x0f0000,
};

#define VM_FAULT_ERROR (VM_FAULT_OOM | VM_FAULT_SIGBUS | \
                        VM_FAULT_SIGSEGV | VM_FAULT_HWPOISON | \
                        VM_FAULT_HWPOISON_LARGE | VM_FAULT_FALLBACK)

struct mm_struct {
    struct vm_area_struct *mmap;    /* list of VMAs */
    struct rb_root mm_rb;
    pgd_t *pgd;
    unsigned long saved_auxv[AT_VECTOR_SIZE]; /* for /proc/PID/auxv */
    unsigned long total_vm;     /* Total pages mapped */
    unsigned long stack_vm;     /* VM_STACK */
    unsigned long data_vm;      /* VM_WRITE & ~VM_SHARED & ~VM_STACK */

    unsigned long mmap_base;    /* base of mmap area */

    /* store ref to file /proc/<pid>/exe symlink points to */
    struct file *exe_file;

    unsigned long task_size;        /* size of task vm space */
    unsigned long start_code, end_code, start_data, end_data;
    unsigned long start_brk, brk, start_stack;
    unsigned long arg_start, arg_end, env_start, env_end;
    unsigned long def_flags;

    struct linux_binfmt *binfmt;

    unsigned long highest_vm_end;   /* highest vma end address */

    unsigned long (*get_unmapped_area)(struct file *filp,
                                       unsigned long addr,
                                       unsigned long len,
                                       unsigned long pgoff,
                                       unsigned long flags);
};

struct vm_fault {
    struct vm_area_struct *vma; /* Target VMA */
    unsigned int flags;         /* FAULT_FLAG_xxx flags */
    pgoff_t pgoff;              /* Logical page offset based on vma */
    unsigned long address;      /* Faulting virtual address */
    pmd_t *pmd; /* Pointer to pmd entry matching the 'address' */
    pte_t *pte; /* Pointer to pte entry matching the 'address'.
                   NULL if the page table hasn't been allocated. */
    pte_t orig_pte;         /* Value of PTE at the time of fault */
    struct page *cow_page;  /* Page handler may use for COW fault */
    struct page *page;      /* ->fault handlers should return a
                             * page here, unless VM_FAULT_NOPAGE
                             * is set (which is also implied by
                             * VM_FAULT_ERROR).
                             */
    pgtable_t prealloc_pte; /* Pre-allocated pte page table.
                             * vm_ops->map_pages() calls
                             * alloc_set_pte() from atomic context.
                             * do_fault_around() pre-allocates
                             * page table to avoid allocation from
                             * atomic context.
                             */
};

struct vm_operations_struct {
    vm_fault_t (*fault)(struct vm_fault *vmf);
    void (*map_pages)(struct vm_fault *vmf,
                      pgoff_t start_pgoff, pgoff_t end_pgoff);
};

struct vm_area_struct {
    unsigned long vm_start; /* Our start address within vm_mm. */
    unsigned long vm_end;   /* The first byte after our end address
                               within vm_mm. */

    /* linked list of VM areas per task, sorted by address */
    struct vm_area_struct *vm_next, *vm_prev;

    struct rb_node vm_rb;

    /*
     * Largest free memory gap in bytes to the left of this VMA.
     * Either between this VMA and vma->vm_prev, or between one of the
     * VMAs below us in the VMA rbtree and its ->vm_prev. This helps
     * get_unmapped_area find a free area of the right size.
     */
    unsigned long rb_subtree_gap;

    struct mm_struct *vm_mm;    /* The address space we belong to. */

    pgprot_t vm_page_prot;
    unsigned long vm_flags;     /* Flags, see mm.h. */

    unsigned long vm_pgoff;     /* Offset within vm_file in PAGE_SIZE units */

    struct file *vm_file;       /* File we map to (can be NULL). */

    /* Function pointers to deal with this struct. */
    const struct vm_operations_struct *vm_ops;
};

static inline void
set_page_private(struct page *page, unsigned long private)
{
    page->private = private;
}

static inline void
vma_init(struct vm_area_struct *vma, struct mm_struct *mm)
{
    static const struct vm_operations_struct dummy_vm_ops = {};

    memset(vma, 0, sizeof(*vma));
    vma->vm_mm = mm;
    vma->vm_ops = &dummy_vm_ops;
    //INIT_LIST_HEAD(&vma->anon_vma_chain);
}

static inline void vma_set_anonymous(struct vm_area_struct *vma)
{
    vma->vm_ops = NULL;
}

static inline bool vma_is_anonymous(struct vm_area_struct *vma)
{
    return !vma->vm_ops;
}

static inline unsigned long vm_start_gap(struct vm_area_struct *vma)
{
    unsigned long vm_start = vma->vm_start;

    if (vma->vm_flags & VM_GROWSDOWN) {
        vm_start -= stack_guard_gap;
        if (vm_start > vma->vm_start)
            vm_start = 0;
    }
    return vm_start;
}

static inline unsigned long vm_end_gap(struct vm_area_struct *vma)
{
    unsigned long vm_end = vma->vm_end;

    if (vma->vm_flags & VM_GROWSUP) {
        vm_end += stack_guard_gap;
        if (vm_end < vma->vm_end)
            vm_end = -PAGE_SIZE;
    }
    return vm_end;
}

static inline unsigned long vma_pages(struct vm_area_struct *vma)
{
    return (vma->vm_end - vma->vm_start) >> PAGE_SHIFT;
}

unsigned long
do_mmap(struct file *file, unsigned long addr,
        unsigned long len, unsigned long prot,
        unsigned long flags, unsigned long pgoff,
        unsigned long *populate, struct list_head *uf);

unsigned long
vm_mmap(struct file *file, unsigned long addr,
        unsigned long len, unsigned long prot,
        unsigned long flag, unsigned long offset);

unsigned long
vm_mmap_pgoff(struct file *file, unsigned long addr,
              unsigned long len, unsigned long prot,
              unsigned long flag, unsigned long pgoff);

unsigned long
get_unmapped_area(struct file *file, unsigned long addr, unsigned long len,
                  unsigned long pgoff, unsigned long flags);

typedef vm_fault_t
(*handle_mm_fault_t)(struct vm_area_struct *vma, unsigned long address,
                     unsigned int flags, struct pt_regs *regs);
extern handle_mm_fault_t handle_mm_fault;

#endif /* _LINUX_MM_TYPES_H */
