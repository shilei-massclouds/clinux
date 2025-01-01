/* SPDX-License-Identifier: GPL-2.0-only */

#include <pgtable.h>

unsigned long BOOT_PT_SV39[PTRS_PER_PGD]
    __attribute__((section(".data.boot_page_table"), used))
    __aligned(PAGE_SIZE);

void init_mmu() {
    // 0x0000_0000_8000_0000..0x0000_0000_c000_0000, VRWX_GAD, 1G block
    BOOT_PT_SV39[2] = (0x80000 << 10) | 0xef;
    // 0xffff_ffc0_8000_0000..0xffff_ffc0_c000_0000, VRWX_GAD, 1G block
    BOOT_PT_SV39[0x102] = (0x80000 << 10) | 0xef;

    /* Qemu pflash acts as the repository of modules,
     * startup loads modules from it.
     * The pflash is located at 0x22000000 in PA,
     * just setup identity-mapping for the first pgdir temporily. */
    // 0x0000_0000..0x4000_0000, VRW__GAD, 1G block
    BOOT_PT_SV39[0] = (0 << 10) | 0xe7;
}
