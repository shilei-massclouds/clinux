// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2012 Regents of the University of California
 * Copyright (C) 2019 Western Digital Corporation or its affiliates.
 */

//#include <linux/init.h>
#include <linux/mm.h>
//#include <linux/memblock.h>
//#include <linux/initrd.h>
//#include <linux/swap.h>
//#include <linux/sizes.h>
//#include <linux/of_fdt.h>
//#include <linux/libfdt.h>
//#include <linux/set_memory.h>
//
#include <asm/fixmap.h>
//#include <asm/tlbflush.h>
#include <asm/sections.h>
//#include <asm/soc.h>
//#include <asm/io.h>
//#include <asm/ptdump.h>
//
//#include "../kernel/head.h"

#include "booter.h"

//
//unsigned long empty_zero_page[PAGE_SIZE / sizeof(unsigned long)]
//							__page_aligned_bss;
//EXPORT_SYMBOL(empty_zero_page);
//
extern char _start[];
void *dtb_early_va;
EXPORT_SYMBOL(dtb_early_va);
//
//static void __init zone_sizes_init(void)
//{
//	unsigned long max_zone_pfns[MAX_NR_ZONES] = { 0, };
//
//#ifdef CONFIG_ZONE_DMA32
//	max_zone_pfns[ZONE_DMA32] = PFN_DOWN(min(4UL * SZ_1G,
//			(unsigned long) PFN_PHYS(max_low_pfn)));
//#endif
//	max_zone_pfns[ZONE_NORMAL] = max_low_pfn;
//
//	free_area_init(max_zone_pfns);
//}
//
//static void setup_zero_page(void)
//{
//	memset((void *)empty_zero_page, 0, PAGE_SIZE);
//}
//
//#if defined(CONFIG_MMU) && defined(CONFIG_DEBUG_VM)
//static inline void print_mlk(char *name, unsigned long b, unsigned long t)
//{
//	pr_notice("%12s : 0x%08lx - 0x%08lx   (%4ld kB)\n", name, b, t,
//		  (((t) - (b)) >> 10));
//}
//
//static inline void print_mlm(char *name, unsigned long b, unsigned long t)
//{
//	pr_notice("%12s : 0x%08lx - 0x%08lx   (%4ld MB)\n", name, b, t,
//		  (((t) - (b)) >> 20));
//}
//
//static void print_vm_layout(void)
//{
//	pr_notice("Virtual kernel memory layout:\n");
//	print_mlk("fixmap", (unsigned long)FIXADDR_START,
//		  (unsigned long)FIXADDR_TOP);
//	print_mlm("pci io", (unsigned long)PCI_IO_START,
//		  (unsigned long)PCI_IO_END);
//	print_mlm("vmemmap", (unsigned long)VMEMMAP_START,
//		  (unsigned long)VMEMMAP_END);
//	print_mlm("vmalloc", (unsigned long)VMALLOC_START,
//		  (unsigned long)VMALLOC_END);
//	print_mlm("lowmem", (unsigned long)PAGE_OFFSET,
//		  (unsigned long)high_memory);
//}
//#else
//static void print_vm_layout(void) { }
//#endif /* CONFIG_DEBUG_VM */
//
//void __init mem_init(void)
//{
//#ifdef CONFIG_FLATMEM
//	BUG_ON(!mem_map);
//#endif /* CONFIG_FLATMEM */
//
//	high_memory = (void *)(__va(PFN_PHYS(max_low_pfn)));
//	memblock_free_all();
//
//	mem_init_print_info(NULL);
//	print_vm_layout();
//}
//
//#ifdef CONFIG_BLK_DEV_INITRD
//static void __init setup_initrd(void)
//{
//	phys_addr_t start;
//	unsigned long size;
//
//	/* Ignore the virtul address computed during device tree parsing */
//	initrd_start = initrd_end = 0;
//
//	if (!phys_initrd_size)
//		return;
//	/*
//	 * Round the memory region to page boundaries as per free_initrd_mem()
//	 * This allows us to detect whether the pages overlapping the initrd
//	 * are in use, but more importantly, reserves the entire set of pages
//	 * as we don't want these pages allocated for other purposes.
//	 */
//	start = round_down(phys_initrd_start, PAGE_SIZE);
//	size = phys_initrd_size + (phys_initrd_start - start);
//	size = round_up(size, PAGE_SIZE);
//
//	if (!memblock_is_region_memory(start, size)) {
//		pr_err("INITRD: 0x%08llx+0x%08lx is not a memory region",
//		       (u64)start, size);
//		goto disable;
//	}
//
//	if (memblock_is_region_reserved(start, size)) {
//		pr_err("INITRD: 0x%08llx+0x%08lx overlaps in-use memory region\n",
//		       (u64)start, size);
//		goto disable;
//	}
//
//	memblock_reserve(start, size);
//	/* Now convert initrd to virtual addresses */
//	initrd_start = (unsigned long)__va(phys_initrd_start);
//	initrd_end = initrd_start + phys_initrd_size;
//	initrd_below_start_ok = 1;
//
//	pr_info("Initial ramdisk at: 0x%p (%lu bytes)\n",
//		(void *)(initrd_start), size);
//	return;
//disable:
//	pr_cont(" - disabling initrd\n");
//	initrd_start = 0;
//	initrd_end = 0;
//}
//#endif /* CONFIG_BLK_DEV_INITRD */
//
phys_addr_t dtb_early_pa __initdata;
EXPORT_SYMBOL(dtb_early_pa);
#ifdef CONFIG_MMU
unsigned long va_pa_offset;
EXPORT_SYMBOL(va_pa_offset);
unsigned long pfn_base;
EXPORT_SYMBOL(pfn_base);

pgd_t swapper_pg_dir[PTRS_PER_PGD] __page_aligned_bss;
EXPORT_SYMBOL(swapper_pg_dir);
pgd_t trampoline_pg_dir[PTRS_PER_PGD] __page_aligned_bss;
pte_t fixmap_pte[PTRS_PER_PTE] __page_aligned_bss;
EXPORT_SYMBOL(fixmap_pte);

#define MAX_EARLY_MAPPING_SIZE	SZ_128M

pgd_t early_pg_dir[PTRS_PER_PGD] __initdata __aligned(PAGE_SIZE);

//void __set_fixmap(enum fixed_addresses idx, phys_addr_t phys, pgprot_t prot)
//{
//	unsigned long addr = __fix_to_virt(idx);
//	pte_t *ptep;
//
//	BUG_ON(idx <= FIX_HOLE || idx >= __end_of_fixed_addresses);
//
//	ptep = &fixmap_pte[pte_index(addr)];
//
//	if (pgprot_val(prot))
//		set_pte(ptep, pfn_pte(phys >> PAGE_SHIFT, prot));
//	else
//		pte_clear(&init_mm, addr, ptep);
//	local_flush_tlb_page(addr);
//}

static pte_t *__init get_pte_virt(phys_addr_t pa)
{
    return (pte_t *)((uintptr_t)pa);
}

static void __init create_pte_mapping(pte_t *ptep,
				      uintptr_t va, phys_addr_t pa,
				      phys_addr_t sz, pgprot_t prot)
{
	uintptr_t pte_idx = pte_index(va);

	BUG_ON(sz != PAGE_SIZE);

	if (pte_none(ptep[pte_idx]))
		ptep[pte_idx] = pfn_pte(PFN_DOWN(pa), prot);
}

#ifndef __PAGETABLE_PMD_FOLDED

pmd_t trampoline_pmd[PTRS_PER_PMD] __page_aligned_bss;
pmd_t fixmap_pmd[PTRS_PER_PMD] __page_aligned_bss;
EXPORT_SYMBOL(fixmap_pmd);

#if MAX_EARLY_MAPPING_SIZE < PGDIR_SIZE
#define NUM_EARLY_PMDS		1UL
#else
#define NUM_EARLY_PMDS		(1UL + MAX_EARLY_MAPPING_SIZE / PGDIR_SIZE)
#endif
pmd_t early_pmd[PTRS_PER_PMD * NUM_EARLY_PMDS] __initdata __aligned(PAGE_SIZE);
EXPORT_SYMBOL(early_pmd);

static pmd_t *__init get_pmd_virt(phys_addr_t pa)
{
    return (pmd_t *)((uintptr_t)pa);
}

static phys_addr_t __init alloc_pmd(uintptr_t va)
{
	uintptr_t pmd_num;

	pmd_num = (va - PAGE_OFFSET) >> PGDIR_SHIFT;
	BUG_ON(pmd_num >= NUM_EARLY_PMDS);
	return (uintptr_t)&early_pmd[pmd_num * PTRS_PER_PMD];
}

static void __init create_pmd_mapping(pmd_t *pmdp,
				      uintptr_t va, phys_addr_t pa,
				      phys_addr_t sz, pgprot_t prot)
{
	pte_t *ptep;
	phys_addr_t pte_phys;
	uintptr_t pmd_idx = pmd_index(va);

	if (sz == PMD_SIZE) {
		if (pmd_none(pmdp[pmd_idx]))
			pmdp[pmd_idx] = pfn_pmd(PFN_DOWN(pa), prot);
		return;
	}

	if (pmd_none(pmdp[pmd_idx])) {
        booter_panic("Cannot alloc pte in booter!");
	} else {
		pte_phys = PFN_PHYS(_pmd_pfn(pmdp[pmd_idx]));
		ptep = get_pte_virt(pte_phys);
	}

	create_pte_mapping(ptep, va, pa, sz, prot);
}

#define pgd_next_t		pmd_t
#define alloc_pgd_next(__va)	alloc_pmd(__va)
#define get_pgd_next_virt(__pa)	get_pmd_virt(__pa)
#define create_pgd_next_mapping(__nextp, __va, __pa, __sz, __prot)	\
	create_pmd_mapping(__nextp, __va, __pa, __sz, __prot)
#define fixmap_pgd_next		fixmap_pmd
#else
#define pgd_next_t		pte_t
#define alloc_pgd_next(__va)	alloc_pte(__va)
#define get_pgd_next_virt(__pa)	get_pte_virt(__pa)
#define create_pgd_next_mapping(__nextp, __va, __pa, __sz, __prot)	\
	create_pte_mapping(__nextp, __va, __pa, __sz, __prot)
#define fixmap_pgd_next		fixmap_pte
#endif

static void __init create_pgd_mapping(pgd_t *pgdp,
				      uintptr_t va, phys_addr_t pa,
				      phys_addr_t sz, pgprot_t prot)
{
	pgd_next_t *nextp;
	phys_addr_t next_phys;
	uintptr_t pgd_idx = pgd_index(va);

	if (sz == PGDIR_SIZE) {
		if (pgd_val(pgdp[pgd_idx]) == 0)
			pgdp[pgd_idx] = pfn_pgd(PFN_DOWN(pa), prot);
		return;
	}

	if (pgd_val(pgdp[pgd_idx]) == 0) {
        next_phys = alloc_pgd_next(va);
        pgdp[pgd_idx] = pfn_pgd(PFN_DOWN(next_phys), PAGE_TABLE);
        nextp = get_pgd_next_virt(next_phys);
        memset(nextp, 0, PAGE_SIZE);
	} else {
		next_phys = PFN_PHYS(_pgd_pfn(pgdp[pgd_idx]));
		nextp = get_pgd_next_virt(next_phys);
	}

	create_pgd_next_mapping(nextp, va, pa, sz, prot);
}

static uintptr_t __init best_map_size(phys_addr_t base, phys_addr_t size)
{
	/* Upgrade to PMD_SIZE mappings whenever possible */
	if ((base & (PMD_SIZE - 1)) || (size & (PMD_SIZE - 1)))
		return PAGE_SIZE;

	return PMD_SIZE;
}

/*
 * setup_vm() is called from head.S with MMU-off.
 *
 * Following requirements should be honoured for setup_vm() to work
 * correctly:
 * 1) It should use PC-relative addressing for accessing kernel symbols.
 *    To achieve this we always use GCC cmodel=medany.
 * 2) The compiler instrumentation for FTRACE will not work for setup_vm()
 *    so disable compiler instrumentation when FTRACE is enabled.
 *
 * Currently, the above requirements are honoured by using custom CFLAGS
 * for init.o in mm/Makefile.
 */

#ifndef __riscv_cmodel_medany
#error "setup_vm() is called from head.S before relocate so it should not use absolute addressing."
#endif

asmlinkage void __init setup_vm(uintptr_t dtb_pa)
{
	uintptr_t va, end_va;
	uintptr_t load_pa = (uintptr_t)(&_start);
	uintptr_t load_sz = (uintptr_t)(&_end) - load_pa;
	uintptr_t map_size = best_map_size(load_pa, MAX_EARLY_MAPPING_SIZE);

	va_pa_offset = PAGE_OFFSET - load_pa;
	pfn_base = PFN_DOWN(load_pa);

	/*
	 * Enforce boot alignment requirements of RV32 and
	 * RV64 by only allowing PMD or PGD mappings.
	 */
	BUG_ON(map_size == PAGE_SIZE);

	/* Sanity check alignment and size */
	BUG_ON((PAGE_OFFSET % PGDIR_SIZE) != 0);
	BUG_ON((load_pa % map_size) != 0);
	BUG_ON(load_sz > MAX_EARLY_MAPPING_SIZE);

	/* Setup early PGD for fixmap */
	create_pgd_mapping(early_pg_dir, FIXADDR_START,
			   (uintptr_t)fixmap_pgd_next, PGDIR_SIZE, PAGE_TABLE);

#ifndef __PAGETABLE_PMD_FOLDED
	/* Setup fixmap PMD */
	create_pmd_mapping(fixmap_pmd, FIXADDR_START,
			   (uintptr_t)fixmap_pte, PMD_SIZE, PAGE_TABLE);
	/* Setup trampoline PGD and PMD */
	create_pgd_mapping(trampoline_pg_dir, PAGE_OFFSET,
			   (uintptr_t)trampoline_pmd, PGDIR_SIZE, PAGE_TABLE);
	create_pmd_mapping(trampoline_pmd, PAGE_OFFSET,
			   load_pa, PMD_SIZE, PAGE_KERNEL_EXEC);
#else
	/* Setup trampoline PGD */
	create_pgd_mapping(trampoline_pg_dir, PAGE_OFFSET,
			   load_pa, PGDIR_SIZE, PAGE_KERNEL_EXEC);
#endif

	/*
	 * Setup early PGD covering entire kernel which will allows
	 * us to reach paging_init(). We map all memory banks later
	 * in setup_vm_final() below.
	 */
	end_va = PAGE_OFFSET + load_sz;
	for (va = PAGE_OFFSET; va < end_va; va += map_size)
		create_pgd_mapping(early_pg_dir, va,
				   load_pa + (va - PAGE_OFFSET),
				   map_size, PAGE_KERNEL_EXEC);

	/* Create fixed mapping for early FDT parsing */
	end_va = __fix_to_virt(FIX_FDT) + FIX_FDT_SIZE;
	for (va = __fix_to_virt(FIX_FDT); va < end_va; va += PAGE_SIZE)
		create_pte_mapping(fixmap_pte, va,
				   dtb_pa + (va - __fix_to_virt(FIX_FDT)),
				   PAGE_SIZE, PAGE_KERNEL);

	/* Save pointer to DTB for early FDT parsing */
	dtb_early_va = (void *)fix_to_virt(FIX_FDT) + (dtb_pa & ~PAGE_MASK);
	/* Save physical address for memblock reservation */
	dtb_early_pa = dtb_pa;
}
#else
asmlinkage void __init setup_vm(uintptr_t dtb_pa)
{
#ifdef CONFIG_BUILTIN_DTB
	dtb_early_va = soc_lookup_builtin_dtb();
	if (!dtb_early_va) {
		/* Fallback to first available DTS */
		dtb_early_va = (void *) __dtb_start;
	}
#else
	dtb_early_va = (void *)dtb_pa;
#endif
	dtb_early_pa = dtb_pa;
}
#endif /* CONFIG_MMU */
//
//#ifdef CONFIG_STRICT_KERNEL_RWX
//void mark_rodata_ro(void)
//{
//	unsigned long text_start = (unsigned long)_text;
//	unsigned long text_end = (unsigned long)_etext;
//	unsigned long rodata_start = (unsigned long)__start_rodata;
//	unsigned long data_start = (unsigned long)_data;
//	unsigned long max_low = (unsigned long)(__va(PFN_PHYS(max_low_pfn)));
//
//	set_memory_ro(text_start, (text_end - text_start) >> PAGE_SHIFT);
//	set_memory_ro(rodata_start, (data_start - rodata_start) >> PAGE_SHIFT);
//	set_memory_nx(rodata_start, (data_start - rodata_start) >> PAGE_SHIFT);
//	set_memory_nx(data_start, (max_low - data_start) >> PAGE_SHIFT);
//
//	debug_checkwx();
//}
//#endif
//
//static void __init resource_init(void)
//{
//	struct memblock_region *region;
//
//	for_each_memblock(memory, region) {
//		struct resource *res;
//
//		res = memblock_alloc(sizeof(struct resource), SMP_CACHE_BYTES);
//		if (!res)
//			panic("%s: Failed to allocate %zu bytes\n", __func__,
//			      sizeof(struct resource));
//
//		if (memblock_is_nomap(region)) {
//			res->name = "reserved";
//			res->flags = IORESOURCE_MEM;
//		} else {
//			res->name = "System RAM";
//			res->flags = IORESOURCE_SYSTEM_RAM | IORESOURCE_BUSY;
//		}
//		res->start = __pfn_to_phys(memblock_region_memory_base_pfn(region));
//		res->end = __pfn_to_phys(memblock_region_memory_end_pfn(region)) - 1;
//
//		request_resource(&iomem_resource, res);
//	}
//}
//
//void __init paging_init(void)
//{
//	setup_vm_final();
//	sparse_init();
//	setup_zero_page();
//	zone_sizes_init();
//	resource_init();
//}
//
//#ifdef CONFIG_SPARSEMEM_VMEMMAP
//int __meminit vmemmap_populate(unsigned long start, unsigned long end, int node,
//			       struct vmem_altmap *altmap)
//{
//	return vmemmap_populate_basepages(start, end, node, NULL);
//}
//#endif
