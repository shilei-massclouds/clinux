/* SPDX-License-Identifier: GPL-2.0-only */

#include <page.h>
#include <csr.h>

#define RISCV_SZPTR     8

#define RISCV_HEADER_VERSION    2
#define RISCV_TEXT_OFFSET       0x200000
#define RISCV_IMAGE_MAGIC       "RISCV\0\0\0"
#define RISCV_IMAGE_MAGIC2      "RSC\x05"

.section .head.text, "ax"
.globl _start
_start:
    /*
     * Image header expected by Linux boot-loaders. The image header data
     * structure is described in asm/image.h.
     * Do not modify it without modifying the structure and all bootloaders
     * that expects this header format!!
     */

    /* jump to start kernel */
    j _start_kernel                 /* code0 */
    .word  0                        /* code1 */
    .dword RISCV_TEXT_OFFSET        /* text_offset: Image load offset */
    .dword _data_end - _start       /* image_size: image size */
    .dword 0                        /* flags: kernel flags */
    .word  RISCV_HEADER_VERSION     /* version */
    .word  0                        /* res1 */
    .dword 0                        /* res2 */
    .ascii RISCV_IMAGE_MAGIC        /* magic */
    .ascii RISCV_IMAGE_MAGIC2       /* magic2 */
    .word  0                        /* res3 */

.align 2
relocate:
    # Distance between va and pa
    li a1, PAGE_OFFSET
    la a2, _start
    sub a1, a1, a2

    # Relocate return address of this function itself
    add ra, ra, a1

    # Point stvec to correspoinding va of that exact position
    la a2, 1f
    add a2, a2, a1
    csrw stvec, a2

    # Prepare for paging
    la a0, early_pgd
    srl a0, a0, PAGE_SHIFT
    li a1, SATP_MODE
    or a0, a0, a1
    sfence.vma
    csrw satp, a0

    # Switch point for paging
.align 2
1:
    la a2, .Lforever_park
    csrw stvec, a2

.option push
.option norelax
    la gp, __global_pointer$
.option pop

    ret

.Lforever_park:
    wfi
    j .Lforever_park

.align 2
setup_trap_vector:
    /* Set trap vector to exception handler */
    la a0, handle_exception
    csrw stvec, a0

    /*
     * Set sup0 scratch register to 0, indicating to exception vector that
     * we are presently executing in kernel.
     */
    csrw sscratch, zero
    ret

.section .init.text, "ax"
_start_kernel:
    /* Mask all interrupts */
    csrw sie, zero
    csrw sip, zero

    /* Load the global pointer */
.option push
.option norelax
    la gp, __global_pointer$
.option pop

    /*
     * Disable FPU to detect illegal usage of
     * floating point in kernel space
     */
    li t0, SR_FS
    csrc sstatus, t0

    /* Clear BSS for flat non-ELF images */
    la a3, _bss_start
    la a4, _bss_stop
    ble a4, a3, clear_bss_done
clear_bss:
    sd zero, (a3)
    add a3, a3, RISCV_SZPTR
    blt a3, a4, clear_bss
clear_bss_done:

    /* Save hart ID and DTB physical address */
    mv s0, a0
    mv s1, a1

    la a2, boot_cpu_hartid
    sd a0, (a2)

    /* Init stack for calling C function */
    la sp, init_stack_top

    /* Init early paging */
    mv a0, s1
    call setup_vm_early

    /* Enable paging */
    call relocate

    call setup_trap_vector

    /* Setup init_task */
    la tp, init_task

    /* Restore stack for C function */
    la sp, init_stack_top

    call load_modules

    tail unreachable
