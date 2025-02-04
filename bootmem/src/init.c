// SPDX-License-Identifier: GPL-2.0-only

#include <linux/init.h>
#include <linux/types.h>
#include <linux/export.h>
#include <linux/initrd.h>
#include <linux/mm_types.h>
#include <linux/memblock.h>
#include <linux/of_fdt.h>
#include <asm/sections.h>
#include "../../booter/src/booter.h"
#include "../../early_fdt/src/libfdt.h"

extern char _start[];

/* use the per-pgdat data instead for discontigmem - mbligh */
unsigned long max_mapnr;
EXPORT_SYMBOL(max_mapnr);

static phys_addr_t dtb_early_pa __initdata;

#ifndef INIT_MM_CONTEXT
#define INIT_MM_CONTEXT(name)
#endif

pgd_t swapper_pg_dir[PTRS_PER_PGD] __page_aligned_bss;
EXPORT_SYMBOL(swapper_pg_dir);

/*
 * For dynamically allocated mm_structs, there is a dynamically sized cpumask
 * at the end of the structure, the size of which depends on the maximum CPU
 * number the system can see. That way we allocate only as much memory for
 * mm_cpumask() as needed for the hundreds, or thousands of processes that
 * a system typically runs.
 *
 * Since there is only one init_mm in the entire system, keep it simple
 * and size this cpu_bitmask to NR_CPUS.
 */
struct mm_struct init_mm = {
    .mm_rb      = RB_ROOT,
    .pgd        = swapper_pg_dir,
    .mm_users   = ATOMIC_INIT(2),
    .mm_count   = ATOMIC_INIT(1),
    MMAP_LOCK_INITIALIZER(init_mm)
    .page_table_lock =  __SPIN_LOCK_UNLOCKED(init_mm.page_table_lock),
    .arg_lock   =  __SPIN_LOCK_UNLOCKED(init_mm.arg_lock),
    .mmlist     = LIST_HEAD_INIT(init_mm.mmlist),
    .user_ns    = &init_user_ns,
    .cpu_bitmap = CPU_BITS_NONE,
    INIT_MM_CONTEXT(init_mm)
};
EXPORT_SYMBOL(init_mm);

int
cl_bootmem_init(void)
{
    sbi_puts("module[bootmem]: init begin ...\n");
    sbi_puts("module[bootmem]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_bootmem_init);

void setup_kernel_in_mm(void)
{
    init_mm.start_code = (unsigned long) _stext;
    init_mm.end_code   = (unsigned long) _etext;
    init_mm.end_data   = (unsigned long) _edata;
    init_mm.brk        = (unsigned long) _end;
}
EXPORT_SYMBOL(setup_kernel_in_mm);

#ifdef CONFIG_BLK_DEV_INITRD
void __init setup_initrd(void)
{
	phys_addr_t start;
	unsigned long size;

	/* Ignore the virtul address computed during device tree parsing */
	initrd_start = initrd_end = 0;

	if (!phys_initrd_size)
		return;
	/*
	 * Round the memory region to page boundaries as per free_initrd_mem()
	 * This allows us to detect whether the pages overlapping the initrd
	 * are in use, but more importantly, reserves the entire set of pages
	 * as we don't want these pages allocated for other purposes.
	 */
	start = round_down(phys_initrd_start, PAGE_SIZE);
	size = phys_initrd_size + (phys_initrd_start - start);
	size = round_up(size, PAGE_SIZE);

	if (!memblock_is_region_memory(start, size)) {
		pr_err("INITRD: 0x%08llx+0x%08lx is not a memory region",
		       (u64)start, size);
		goto disable;
	}

	if (memblock_is_region_reserved(start, size)) {
		pr_err("INITRD: 0x%08llx+0x%08lx overlaps in-use memory region\n",
		       (u64)start, size);
		goto disable;
	}

	memblock_reserve(start, size);
	/* Now convert initrd to virtual addresses */
	initrd_start = (unsigned long)__va(phys_initrd_start);
	initrd_end = initrd_start + phys_initrd_size;
	initrd_below_start_ok = 1;

	pr_info("Initial ramdisk at: 0x%p (%lu bytes)\n",
		(void *)(initrd_start), size);
	return;
disable:
	pr_cont(" - disabling initrd\n");
	initrd_start = 0;
	initrd_end = 0;
    //booter_panic("No impl 'setup_initrd'.");
}
#endif

void __init setup_bootmem(void)
{
	struct memblock_region *reg;
	phys_addr_t mem_size = 0;
	phys_addr_t total_mem = 0;
	phys_addr_t mem_start, end = 0;
	phys_addr_t vmlinux_end = __pa_symbol(&_end);
	phys_addr_t vmlinux_start = __pa_symbol(&_start);

	/* Find the memory region containing the kernel */
	for_each_memblock(memory, reg) {
		end = reg->base + reg->size;
		if (!total_mem)
			mem_start = reg->base;
		if (reg->base <= vmlinux_start && vmlinux_end <= end)
			BUG_ON(reg->size == 0);
		total_mem = total_mem + reg->size;
	}

	/*
	 * Remove memblock from the end of usable area to the
	 * end of region
	 */
	mem_size = min(total_mem, (phys_addr_t)-PAGE_OFFSET);
	if (mem_start + mem_size < end)
		memblock_remove(mem_start + mem_size,
				end - mem_start - mem_size);

	/* Reserve from the start of the kernel to the end of the kernel */
	memblock_reserve(vmlinux_start, vmlinux_end - vmlinux_start);

	max_pfn = PFN_DOWN(memblock_end_of_DRAM());
	max_low_pfn = max_pfn;
	set_max_mapnr(max_low_pfn);

#ifdef CONFIG_BLK_DEV_INITRD
	setup_initrd();
#endif /* CONFIG_BLK_DEV_INITRD */

	/*
	 * Avoid using early_init_fdt_reserve_self() since __pa() does
	 * not work for DTB pointers that are fixmap addresses
	 */
	memblock_reserve(dtb_early_pa, fdt_totalsize(dtb_early_va));

	early_init_fdt_scan_reserved_mem();
	memblock_allow_resize();
	memblock_dump_all();

	for_each_memblock(memory, reg) {
		unsigned long start_pfn = memblock_region_memory_base_pfn(reg);
		unsigned long end_pfn = memblock_region_memory_end_pfn(reg);

		memblock_set_node(PFN_PHYS(start_pfn),
				  PFN_PHYS(end_pfn - start_pfn),
				  &memblock.memory, 0);
	}
}
EXPORT_SYMBOL(setup_bootmem);
