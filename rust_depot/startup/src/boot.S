/* SPDX-License-Identifier: GPL-2.0-only */

#include <page.h>
#include <csr.h>

.section .text.boot, "ax"
.globl _start
_start:
    // PC = 0x8020_0000
    // a0 = hartid
    mv      s0, a0              // 0. save hartid
    la      sp, boot_stack_top  // 1. set SP

    call    init_mmu            // 2. setup boot page table and enabel MMU

    # Prepare for paging
    la a0, BOOT_PT_SV39
    srl a0, a0, PAGE_SHIFT
    li a1, SATP_MODE
    or a0, a0, a1
    sfence.vma
    csrw satp, a0
    sfence.vma

    // NOW: const PHYS_VIRT_OFFSET doesn't work here. Why?
    li      t1, 0xffffffc000000000  // fixit: Use config
    la      t0, 1f
    add     t0, t0, t1
    jr      t0
1:                              // jump point from pa to va

    la      sp, boot_stack_top  // *. reset SP

    /* Clear BSS for flat non-ELF images */
    la a3, sbss
    la a4, ebss
    ble a4, a3, clear_bss_done
clear_bss:
    sd zero, (a3)
    add a3, a3, 8
    blt a3, a4, clear_bss
clear_bss_done:

    call load_modules

    tail unreachable
