/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef _RISCV_MM_H_
#define _RISCV_MM_H_

#include <page.h>
#include <errno.h>
#include <atomic.h>
#include <string.h>
#include <mmzone.h>
#include <ptrace.h>
#include <pgtable.h>
#include <memblock.h>
#include <resource.h>
#include <page-flags.h>
#include <rbtree.h>
#include <rmap.h>

#define untagged_addr(addr) (addr)

/* Page flags: | ZONE | [LAST_CPUPID] | ... | FLAGS | */
#define NODES_PGOFF     (sizeof(unsigned long)*8)
#define ZONES_PGOFF     (NODES_PGOFF - ZONES_WIDTH)

#define ZONEID_SHIFT    ZONES_SHIFT
#define ZONEID_PGOFF    ZONES_PGOFF
#define ZONEID_PGSHIFT  ZONEID_PGOFF

#define ZONES_PGSHIFT   ZONES_PGOFF
#define ZONES_MASK      ((1UL << ZONES_WIDTH) - 1)
#define ZONEID_MASK     ((1UL << ZONEID_SHIFT) - 1)

#define page_address(page)  lowmem_page_address(page)

#define offset_in_page(p)   ((unsigned long)(p) & ~PAGE_MASK)

#define ALLOC_WMARK_LOW     WMARK_LOW

/*
 * vm_flags in vm_area_struct, see mm_types.h.
 * When changing, update also include/trace/events/mmflags.h
 */
#define VM_NONE         0x00000000
#define VM_READ         0x00000001  /* currently active flags */
#define VM_WRITE        0x00000002
#define VM_EXEC         0x00000004
#define VM_SHARED       0x00000008

#define VM_MAYREAD      0x00000010  /* limits for mprotect() etc */
#define VM_MAYWRITE     0x00000020
#define VM_MAYEXEC      0x00000040
#define VM_MAYSHARE     0x00000080

#define VM_GROWSDOWN    0x00000100  /* general info on the segment */
#define VM_DENYWRITE    0x00000800  /* ETXTBSY on write attempts.. */
#define VM_LOCKED       0x00002000
#define VM_GROWSUP      VM_NONE
#define VM_SEQ_READ     0x00008000  /* App will access data sequentially */
#define VM_RAND_READ    0x00010000  /* App will not benefit from clustered reads */
#define VM_ACCOUNT      0x00100000  /* Is a VM accounted object */
#define VM_NORESERVE    0x00200000  /* should the VM suppress accounting */
#define VM_SYNC         0x00800000  /* Synchronous page faults */

/* Bits set in the VMA until the stack is in its final location */
#define VM_STACK_INCOMPLETE_SETUP   (VM_RAND_READ | VM_SEQ_READ)

#define VM_DATA_FLAGS_NON_EXEC \
    (VM_READ | VM_WRITE | VM_MAYREAD | VM_MAYWRITE | VM_MAYEXEC)

#define VM_DATA_DEFAULT_FLAGS   VM_DATA_FLAGS_NON_EXEC
#define VM_STACK_DEFAULT_FLAGS  VM_DATA_DEFAULT_FLAGS

#define VM_STACK        VM_GROWSDOWN

#define VM_STACK_FLAGS  (VM_STACK | VM_STACK_DEFAULT_FLAGS | VM_ACCOUNT)

#define FOLL_WRITE  0x01    /* check pte is writable */
#define FOLL_TOUCH  0x02    /* mark page accessed */
#define FOLL_GET    0x04    /* do get_page on page */
#define FOLL_FORCE  0x10    /* get_user_pages read/write w/o permission */
#define FOLL_NOWAIT 0x20    /* if a disk transfer is needed, start the IO
                             * and return without waiting upon it */
#define FOLL_POPULATE   0x40    /* fault in page */
#define FOLL_TRIED  0x800   /* a retry, previous pass started an IO */

#define FOLL_MLOCK  0x1000  /* lock present pages */

#define FOLL_REMOTE 0x2000  /* we are working on non-current tsk/mm */

#define FOLL_LONGTERM   0x10000 /* mapping lifetime is indefinite: see below */
#define FOLL_PIN        0x40000 /* pages must be released via unpin_user_page */

#define FAULT_FLAG_WRITE            0x01
#define FAULT_FLAG_ALLOW_RETRY      0x04
#define FAULT_FLAG_RETRY_NOWAIT     0x08
#define FAULT_FLAG_KILLABLE         0x10
#define FAULT_FLAG_TRIED            0x20
#define FAULT_FLAG_USER             0x40
#define FAULT_FLAG_REMOTE           0x80
#define FAULT_FLAG_INTERRUPTIBLE    0x200

/*
 * The default fault flags that should be used by most of the
 * arch-specific page fault handlers.
 */
#define FAULT_FLAG_DEFAULT \
    (FAULT_FLAG_ALLOW_RETRY | \
     FAULT_FLAG_KILLABLE | \
     FAULT_FLAG_INTERRUPTIBLE)

extern struct mm_struct init_mm;

struct alloc_context {
    struct zonelist *zonelist;
    struct zoneref *preferred_zoneref;

    /*
     * highest_zoneidx represents highest usable zone index of
     * the allocation request. Due to the nature of the zone,
     * memory on lower zone than the highest_zoneidx will be
     * protected by lowmem_reserve[highest_zoneidx].
     *
     * highest_zoneidx is also used by reclaim/compaction to limit
     * the target zone since higher zone than this index cannot be
     * usable for this allocation request.
     */
    enum zone_type highest_zoneidx;
};

struct vm_unmapped_area_info {
#define VM_UNMAPPED_AREA_TOPDOWN 1
    unsigned long flags;
    unsigned long length;
    unsigned long low_limit;
    unsigned long high_limit;
    unsigned long align_mask;
    unsigned long align_offset;
};

extern unsigned long max_mapnr;

static inline void set_max_mapnr(unsigned long limit)
{
    max_mapnr = limit;
}

extern atomic_long_t _totalram_pages;

static inline void totalram_pages_add(long count)
{
    atomic_long_add(count, &_totalram_pages);
}

static __always_inline void *
lowmem_page_address(const struct page *page)
{
    return page_to_virt(page);
}

extern pgd_t early_pgd[];
extern pmd_t early_pmd[];
extern pmd_t fixmap_pmd[];
extern pte_t fixmap_pt[];
extern pgd_t swapper_pg_dir[];

extern phys_addr_t dtb_early_pa;

typedef phys_addr_t (*phys_alloc_t)(phys_addr_t size, phys_addr_t align);

typedef void (*do_page_fault_t)(struct pt_regs *);

void
setup_fixmap_pgd(void);

void
setup_vm_final(struct memblock_region *regions,
               unsigned long regions_cnt,
               phys_alloc_t alloc);

void
clear_flash_pgd(void);

const char *
kstrdup_const(const char *s, gfp_t gfp);

char *
kstrdup(const char *s, gfp_t gfp);

void
kfree_const(const void *x);

static inline enum zone_type
page_zonenum(const struct page *page)
{
    return (page->flags >> ZONES_PGSHIFT) & ZONES_MASK;
}

static inline struct zone *
page_zone(const struct page *page)
{
    return &NODE_DATA(0)->node_zones[page_zonenum(page)];
}

/*
 * Locate the struct page for both the matching buddy in our
 * pair (buddy1) and the combined O(n+1) page they form (page).
 *
 * 1) Any buddy B1 will have an order O twin B2 which satisfies
 * the following equation:
 *     B2 = B1 ^ (1 << O)
 * For example, if the starting buddy (buddy2) is #8 its order
 * 1 buddy is #10:
 *     B2 = 8 ^ (1 << 1) = 8 ^ 2 = 10
 *
 * 2) Any buddy B will have an order O+1 parent P which
 * satisfies the following equation:
 *     P = B & ~(1 << O)
 *
 * Assumption: *_mem_map is contiguous at least up to MAX_ORDER
 */
static inline unsigned long
__find_buddy_pfn(unsigned long page_pfn, unsigned int order)
{
    return page_pfn ^ (1 << order);
}

static inline unsigned int
page_order(struct page *page)
{
    /* PageBuddy() must be checked by the caller */
    return page_private(page);
}

static inline int page_zone_id(struct page *page)
{
    return (page->flags >> ZONEID_PGSHIFT) & ZONEID_MASK;
}

static inline struct page *virt_to_head_page(const void *x)
{
    struct page *page = virt_to_page(x);

    return compound_head(page);
}

int insert_vm_struct(struct mm_struct *mm, struct vm_area_struct *vma);

void __vma_link_list(struct mm_struct *mm, struct vm_area_struct *vma,
                     struct vm_area_struct *prev);

extern unsigned long stack_guard_gap;

long get_user_pages_remote(struct mm_struct *mm,
                           unsigned long start, unsigned long nr_pages,
                           unsigned int gup_flags, struct page **pages,
                           struct vm_area_struct **vmas, int *locked);

struct vm_area_struct *
find_extend_vma(struct mm_struct *mm, unsigned long addr);

int __pmd_alloc(struct mm_struct *mm, pgd_t *pgd, unsigned long address);

static inline pmd_t *
pmd_alloc(struct mm_struct *mm, pgd_t *pgd, unsigned long address)
{
    return (unlikely(pgd_none(*pgd)) && __pmd_alloc(mm, pgd, address)) ?
        NULL: pmd_offset(pgd, address);
}

int __pte_alloc(struct mm_struct *mm, pmd_t *pmd);

#define pte_alloc(mm, pmd) \
    (unlikely(pmd_none(*(pmd))) && __pte_alloc(mm, pmd))

#define pte_offset_map(dir, address) \
    pte_offset_kernel((dir), (address))

#define pte_offset_map_lock(mm, pmd, address) \
({                          \
    pte_t *__pte = pte_offset_map(pmd, address);    \
    __pte;                      \
})

pgprot_t vm_get_page_prot(unsigned long vm_flags);

int set_page_dirty(struct page *page);

int expand_stack(struct vm_area_struct *vma, unsigned long address);

extern int
__mm_populate(unsigned long addr, unsigned long len, int ignore_errors);

static inline void mm_populate(unsigned long addr, unsigned long len)
{
    /* Ignore errors */
    (void) __mm_populate(addr, len, 1);
}

void
arch_pick_mmap_layout(struct mm_struct *mm, struct rlimit *rlim_stack);

int vm_brk_flags(unsigned long addr, unsigned long request,
                 unsigned long flags);

struct vm_area_struct *
find_vma(struct mm_struct *mm, unsigned long addr);

static inline int
check_data_rlimit(unsigned long rlim,
                  unsigned long new, unsigned long start,
                  unsigned long end_data, unsigned long start_data)
{
    if (rlim < RLIM_INFINITY) {
        if (((new - start) + (end_data - start_data)) > rlim)
            return -ENOSPC;
    }

    return 0;
}

int do_brk_flags(unsigned long addr, unsigned long len,
                 unsigned long flags, struct list_head *uf);

static inline bool want_init_on_alloc(gfp_t flags)
{
    return flags & __GFP_ZERO;
}

char *strndup_user(const char *s, long n);

extern int vm_munmap(unsigned long, size_t);

extern int do_munmap(struct mm_struct *, unsigned long, size_t,
                     struct list_head *uf);

extern int
__vma_adjust(struct vm_area_struct *vma,
             unsigned long start, unsigned long end,
             pgoff_t pgoff, struct vm_area_struct *insert,
             struct vm_area_struct *expand);

static inline int
vma_adjust(struct vm_area_struct *vma,
           unsigned long start, unsigned long end,
           pgoff_t pgoff, struct vm_area_struct *insert)
{
    return __vma_adjust(vma, start, end, pgoff, insert, NULL);
}

/* interval_tree.c */
void vma_interval_tree_insert(struct vm_area_struct *node,
                  struct rb_root_cached *root);
void vma_interval_tree_insert_after(struct vm_area_struct *node,
                    struct vm_area_struct *prev,
                    struct rb_root_cached *root);
void vma_interval_tree_remove(struct vm_area_struct *node,
                  struct rb_root_cached *root);
struct vm_area_struct *vma_interval_tree_iter_first(struct rb_root_cached *root,
                unsigned long start, unsigned long last);
struct vm_area_struct *vma_interval_tree_iter_next(struct vm_area_struct *node,
                unsigned long start, unsigned long last);

#define vma_interval_tree_foreach(vma, root, start, last)       \
    for (vma = vma_interval_tree_iter_first(root, start, last); \
         vma; vma = vma_interval_tree_iter_next(vma, start, last))

void anon_vma_interval_tree_insert(struct anon_vma_chain *node,
                   struct rb_root_cached *root);
void anon_vma_interval_tree_remove(struct anon_vma_chain *node,
                   struct rb_root_cached *root);
struct anon_vma_chain *
anon_vma_interval_tree_iter_first(struct rb_root_cached *root,
                  unsigned long start, unsigned long last);
struct anon_vma_chain *anon_vma_interval_tree_iter_next(
    struct anon_vma_chain *node, unsigned long start, unsigned long last);
#ifdef CONFIG_DEBUG_VM_RB
void anon_vma_interval_tree_verify(struct anon_vma_chain *node);
#endif

#define anon_vma_interval_tree_foreach(avc, root, start, last)       \
    for (avc = anon_vma_interval_tree_iter_first(root, start, last); \
         avc; avc = anon_vma_interval_tree_iter_next(avc, start, last))

/*
static inline void update_hiwater_vm(struct mm_struct *mm)
{
    if (mm->hiwater_vm < mm->total_vm)
        mm->hiwater_vm = mm->total_vm;
}
*/

extern unsigned long vm_unmapped_area(struct vm_unmapped_area_info *info);

struct file;

extern unsigned long
arch_get_unmapped_area(struct file *, unsigned long, unsigned long,
                       unsigned long, unsigned long);

extern unsigned long
arch_get_unmapped_area_topdown(struct file *filp, unsigned long addr,
                               unsigned long len, unsigned long pgoff,
                               unsigned long flags);

int __do_munmap(struct mm_struct *mm, unsigned long start, size_t len,
                struct list_head *uf, bool downgrade);

void mark_page_accessed(struct page *page);

void create_pgd_mapping(pgd_t *pgdp,
                        uintptr_t va, phys_addr_t pa,
                        phys_addr_t sz, pgprot_t prot);

void reset_phys_alloc_fn(phys_alloc_t alloc);

typedef long
(*sys_mmap_t)(unsigned long addr, unsigned long len,
              unsigned long prot, unsigned long flags,
              unsigned long fd, off_t offset,
              unsigned long page_shift_offset);
extern sys_mmap_t riscv_sys_mmap;

typedef int
(*do_vm_munmap_t)(unsigned long start, size_t len, bool downgrade);
extern do_vm_munmap_t do_vm_munmap;

#endif /* _RISCV_MM_H_ */
