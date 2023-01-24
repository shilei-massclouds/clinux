/* SPDX-License-Identifier: GPL-2.0-only */

#include <csr.h>

.macro fixup op reg addr lbl
100:
    \op \reg, \addr
    .section __ex_table,"a"
    .balign 8
    .dword 100b, \lbl
    .previous
    .endm

.globl __asm_copy_to_user
.globl __asm_copy_from_user
__asm_copy_to_user:
__asm_copy_from_user:

    /* Enable access to user memory */
    li t6, SR_SUM
    csrs CSR_STATUS, t6

    add a3, a1, a2
    /* Use word-oriented copy only if low-order bits match */
    andi t0, a0, 7
    andi t1, a1, 7
    bne t0, t1, 2f

    addi t0, a1, 7
    andi t1, a3, ~7
    andi t0, t0, ~7
    /*
     * a3: terminal address of source region
     * t0: lowest XLEN-aligned address in source
     * t1: highest XLEN-aligned address in source
     */
    bgeu t0, t1, 2f
    bltu a1, t0, 4f
1:
    fixup ld, t2, (a1), 10f
    fixup sd, t2, (a0), 10f
    addi a1, a1, 8
    addi a0, a0, 8
    bltu a1, t1, 1b
2:
    bltu a1, a3, 5f

3:
    /* Disable access to user memory */
    csrc CSR_STATUS, t6
    li a0, 0
    ret
4: /* Edge case: unalignment */
    fixup lbu, t2, (a1), 10f
    fixup sb, t2, (a0), 10f
    addi a1, a1, 1
    addi a0, a0, 1
    bltu a1, t0, 4b
    j 1b
5: /* Edge case: remainder */
    fixup lbu, t2, (a1), 10f
    fixup sb, t2, (a0), 10f
    addi a1, a1, 1
    addi a0, a0, 1
    bltu a1, a3, 5b
    j 3b

.section .fixup,"ax"
.balign 4
/* Fixup code for __copy_user(10) and __clear_user(11) */
10:
    /* Disable access to user memory */
    csrs CSR_STATUS, t6
    mv a0, a2
    ret
11:
    csrs CSR_STATUS, t6
    mv a0, a1
    ret
    .previous
