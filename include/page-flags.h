/* SPDX-License-Identifier: GPL-2.0 */
#ifndef PAGE_FLAGS_H
#define PAGE_FLAGS_H

/*
 * Don't use the *_dontuse flags.  Use the macros.  Otherwise you'll break
 * locked- and dirty-page accounting.
 *
 * The page flags field is split into two parts, the main flags area
 * which extends from the low bits upwards, and the fields area which
 * extends from the high bits downwards.
 *
 *  | FIELD | ... | FLAGS |
 *  N-1           ^       0
 *               (NR_PAGEFLAGS)
 *
 * The fields area is reserved for fields mapping zone, node (for NUMA) and
 * SPARSEMEM section (for variants of SPARSEMEM that require section ids like
 * SPARSEMEM_EXTREME with !SPARSEMEM_VMEMMAP).
 */
enum pageflags {
    PG_locked,          /* Page is locked. Don't touch. */
    PG_referenced,
    PG_uptodate,
    PG_dirty,
    PG_lru,
    PG_active,
    PG_workingset,
    PG_waiters,         /* Page has waiters, check its waitqueue. */
    PG_error,
    PG_slab,
    PG_owner_priv_1,    /* Owner use. If pagecache, fs may use*/
    PG_arch_1,
    PG_reserved,
    PG_private,         /* If pagecache, has fs-private data */
    PG_private_2,       /* If pagecache, has fs aux data */
    PG_writeback,       /* Page is under writeback */
    PG_head,            /* A head page */
    PG_mappedtodisk,    /* Has blocks allocated on-disk */
    PG_reclaim,         /* To be reclaimed asap */
    PG_swapbacked,      /* Page is backed by RAM/swap */
    PG_unevictable,     /* Page is "unevictable"  */
    PG_mlocked,         /* Page is vma mlocked */
    __NR_PAGEFLAGS,
};

static inline struct page *compound_head(struct page *page)
{
    unsigned long head = READ_ONCE(page->compound_head);

    if (unlikely(head & 1))
        return (struct page *) (head - 1);
    return page;
}

static __always_inline int PageTail(struct page *page)
{
    return READ_ONCE(page->compound_head) & 1;
}

static __always_inline int PageCompound(struct page *page)
{
    return test_bit(PG_head, &page->flags) || PageTail(page);
}

#define PAGE_POISON_PATTERN -1l
static inline int PagePoisoned(const struct page *page)
{
    return page->flags == PAGE_POISON_PATTERN;
}

/*
 * BUILD_BUG_ON_INVALID() permits the compiler to check the validity of the
 * expression but avoids the generation of any code, even if that expression
 * has side-effects.
 */
#define BUILD_BUG_ON_INVALID(e) ((void)(sizeof((long)(e))))

#define VM_BUG_ON_PGFLAGS(cond, page) BUILD_BUG_ON_INVALID(cond)

/*
 * Page flags policies wrt compound pages
 *
 * PF_POISONED_CHECK
 *     check if this struct page poisoned/uninitialized
 *
 * PF_ANY:
 *     the page flag is relevant for small, head and tail pages.
 *
 * PF_HEAD:
 *     for compound page all operations related to the page flag applied to
 *     head page.
 *
 * PF_ONLY_HEAD:
 *     for compound page, callers only ever operate on the head page.
 *
 * PF_NO_TAIL:
 *     modifications of the page flag must be done on small or head pages,
 *     checks can be done on tail pages too.
 *
 * PF_NO_COMPOUND:
 *     the page flag is not relevant for compound pages.
 */
#define PF_POISONED_CHECK(page)                     \
({                                                  \
    VM_BUG_ON_PGFLAGS(PagePoisoned(page), page);    \
    page;                                           \
})

#define PF_ANY(page, enforce)   PF_POISONED_CHECK(page)

#define PF_HEAD(page, enforce)  PF_POISONED_CHECK(compound_head(page))

#define PF_NO_TAIL(page, enforce) ({                        \
        VM_BUG_ON_PGFLAGS(enforce && PageTail(page), page); \
        PF_POISONED_CHECK(compound_head(page)); })

#define PF_NO_COMPOUND(page, enforce) ({                \
        VM_BUG_ON_PGFLAGS(enforce && PageCompound(page), page); \
        PF_POISONED_CHECK(page); })

#define TESTPAGEFLAG(uname, lname, policy)                      \
static __always_inline int Page##uname(struct page *page)       \
    { return test_bit(PG_##lname, &policy(page, 0)->flags); }

#define SETPAGEFLAG(uname, lname, policy)                       \
static __always_inline void SetPage##uname(struct page *page)   \
    { set_bit(PG_##lname, &policy(page, 1)->flags); }

#define CLEARPAGEFLAG(uname, lname, policy)                     \
static __always_inline void ClearPage##uname(struct page *page) \
    { clear_bit(PG_##lname, &policy(page, 1)->flags); }

#define TESTCLEARFLAG(uname, lname, policy)                         \
static __always_inline int TestClearPage##uname(struct page *page)  \
    { return test_and_clear_bit(PG_##lname, &policy(page, 1)->flags); }

#define __CLEARPAGEFLAG(uname, lname, policy)                       \
static __always_inline void __ClearPage##uname(struct page *page)   \
    { __clear_bit(PG_##lname, &policy(page, 1)->flags); }

#define __SETPAGEFLAG(uname, lname, policy)                     \
static __always_inline void __SetPage##uname(struct page *page) \
    { __set_bit(PG_##lname, &policy(page, 1)->flags); }

#define PAGEFLAG(uname, lname, policy)  \
    TESTPAGEFLAG(uname, lname, policy)  \
    SETPAGEFLAG(uname, lname, policy)   \
    CLEARPAGEFLAG(uname, lname, policy)

#define __PAGEFLAG(uname, lname, policy)    \
    TESTPAGEFLAG(uname, lname, policy)      \
    __SETPAGEFLAG(uname, lname, policy)     \
    __CLEARPAGEFLAG(uname, lname, policy)

#define TESTSETFLAG(uname, lname, policy)               \
static __always_inline int TestSetPage##uname(struct page *page)    \
    { return test_and_set_bit(PG_##lname, &policy(page, 1)->flags); }

#define TESTSCFLAG(uname, lname, policy)    \
    TESTSETFLAG(uname, lname, policy)       \
    TESTCLEARFLAG(uname, lname, policy)

__PAGEFLAG(Slab, slab, PF_NO_TAIL)

PAGEFLAG(Reserved, reserved, PF_NO_COMPOUND)
    __SETPAGEFLAG(Reserved, reserved, PF_NO_COMPOUND)
    __CLEARPAGEFLAG(Reserved, reserved, PF_NO_COMPOUND)

/*
 * Private page markings that may be used by the filesystem
 * that owns the page for its own purposes.
 */
PAGEFLAG(Private, private, PF_ANY)
    __SETPAGEFLAG(Private, private, PF_ANY)
    __CLEARPAGEFLAG(Private, private, PF_ANY)

PAGEFLAG(Active, active, PF_HEAD)
    __CLEARPAGEFLAG(Active, active, PF_HEAD)
    TESTCLEARFLAG(Active, active, PF_HEAD)

PAGEFLAG(Dirty, dirty, PF_HEAD)
    __CLEARPAGEFLAG(Dirty, dirty, PF_HEAD)
    TESTSCFLAG(Dirty, dirty, PF_HEAD)

PAGEFLAG(Error, error, PF_NO_TAIL)
    TESTCLEARFLAG(Error, error, PF_NO_TAIL)

PAGEFLAG(MappedToDisk, mappedtodisk, PF_NO_TAIL)

PAGEFLAG(Readahead, reclaim, PF_NO_COMPOUND)
    TESTCLEARFLAG(Readahead, reclaim, PF_NO_COMPOUND)

#define PAGE_TYPE_BASE  0xf0000000
/* Reserve      0x0000007f to catch underflows of page_mapcount */
#define PAGE_MAPCOUNT_RESERVE   -128
#define PG_buddy    0x00000080
#define PG_offline  0x00000100
#define PG_kmemcg   0x00000200
#define PG_table    0x00000400
#define PG_guard    0x00000800

#define PageType(page, flag) \
    ((page->page_type & (PAGE_TYPE_BASE | flag)) == PAGE_TYPE_BASE)

static inline int page_has_type(struct page *page)
{
    return (int)page->page_type < PAGE_MAPCOUNT_RESERVE;
}

#define PAGE_TYPE_OPS(uname, lname)                 \
static __always_inline int Page##uname(struct page *page)       \
{                                   \
    return PageType(page, PG_##lname);              \
}                                   \
static __always_inline void __SetPage##uname(struct page *page)     \
{                                   \
    page->page_type &= ~PG_##lname;                 \
}                                   \
static __always_inline void __ClearPage##uname(struct page *page)   \
{                                   \
    page->page_type |= PG_##lname;                  \
}

/*
 * PageBuddy() indicates that the page is free and in the buddy system
 */
PAGE_TYPE_OPS(Buddy, buddy)

/*
 * Marks pages in use as page tables.
 */
PAGE_TYPE_OPS(Table, table)

static inline int PageUptodate(struct page *page)
{
    int ret;
    page = compound_head(page);
    ret = test_bit(PG_uptodate, &(page)->flags);
    return ret;
}

static __always_inline void __SetPageUptodate(struct page *page)
{
    BUG_ON(PageTail(page));
    __set_bit(PG_uptodate, &page->flags);
}

static __always_inline void SetPageUptodate(struct page *page)
{
    BUG_ON(PageTail(page));
    /*
     * Memory barrier must be issued before setting the PG_uptodate bit,
     * so that all previous stores issued in order to bring the page
     * uptodate are actually visible before PageUptodate becomes true.
     */
    set_bit(PG_uptodate, &page->flags);
}

CLEARPAGEFLAG(Uptodate, uptodate, PF_NO_TAIL)

#endif /* PAGE_FLAGS_H */
