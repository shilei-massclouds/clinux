/* SPDX-License-Identifier: GPL-2.0-only */

#include <csr.h>
#include <ptrace.h>
#include <unistd.h>
#include <asm-offsets.h>
#include <thread_info.h>

#define RISCV_PTR       .dword
#define RISCV_LGPTR     3

.align 2
.globl handle_exception
handle_exception:
	csrrw tp, CSR_SSCRATCH, tp
	bnez tp, _save_context

_restore_kernel_tpsp:
	csrr tp, CSR_SSCRATCH
	sd sp, TASK_TI_KERNEL_SP(tp)
_save_context:
	sd sp, TASK_TI_USER_SP(tp)
	ld sp, TASK_TI_KERNEL_SP(tp)
	addi sp, sp, -(PT_SIZE_ON_STACK)
	sd x1,  PT_RA(sp)
	sd x3,  PT_GP(sp)
	sd x5,  PT_T0(sp)
	sd x6,  PT_T1(sp)
	sd x7,  PT_T2(sp)
	sd x8,  PT_S0(sp)
	sd x9,  PT_S1(sp)
	sd x10, PT_A0(sp)
	sd x11, PT_A1(sp)
	sd x12, PT_A2(sp)
	sd x13, PT_A3(sp)
	sd x14, PT_A4(sp)
	sd x15, PT_A5(sp)
	sd x16, PT_A6(sp)
	sd x17, PT_A7(sp)
	sd x18, PT_S2(sp)
	sd x19, PT_S3(sp)
	sd x20, PT_S4(sp)
	sd x21, PT_S5(sp)
	sd x22, PT_S6(sp)
	sd x23, PT_S7(sp)
	sd x24, PT_S8(sp)
	sd x25, PT_S9(sp)
	sd x26, PT_S10(sp)
	sd x27, PT_S11(sp)
	sd x28, PT_T3(sp)
	sd x29, PT_T4(sp)
	sd x30, PT_T5(sp)
	sd x31, PT_T6(sp)

	/*
	 * Disable user-mode memory access as it should only be set in the
	 * actual user copy routines.
	 *
	 * Disable the FPU to detect illegal usage of floating point in kernel
	 * space.
	 */
	li t0, SR_SUM | SR_FS

	ld s0, TASK_TI_USER_SP(tp)
	csrrc s1, CSR_SSTATUS, t0
	csrr s2, CSR_SEPC
	csrr s3, CSR_STVAL
	csrr s4, CSR_SCAUSE
	csrr s5, CSR_SSCRATCH
	sd s0, PT_SP(sp)
	sd s1, PT_STATUS(sp)
	sd s2, PT_EPC(sp)
	sd s3, PT_BADADDR(sp)
	sd s4, PT_CAUSE(sp)
	sd s5, PT_TP(sp)

	/*
	 * Set the scratch register to 0, so that if a recursive exception
	 * occurs, the exception vector knows it came from the kernel
	 */
	csrw CSR_SSCRATCH, x0

	/* Load the global pointer */
.option push
.option norelax
	la gp, __global_pointer$
.option pop

	/*
	 * MSB of cause differentiates between
	 * interrupts and exceptions
	 */
	bge s4, zero, 1f

	la ra, ret_from_exception

	/* Handle interrupts */
	move a0, sp /* pt_regs */
	la a1, handle_arch_irq
	ld a1, (a1)
	jr a1
1:
    /*
     * Exceptions run with interrupts enabled or disabled depending on the
     * state of SR_PIE in m/sstatus.
     */
    andi t0, s1, SR_PIE
    beqz t0, 1f
    csrs CSR_STATUS, SR_IE

1:
    la ra, ret_from_exception
    /* Handle syscalls */
    li t0, EXC_SYSCALL
    beq s4, t0, handle_syscall

    /* Handle other exceptions */
    slli t0, s4, RISCV_LGPTR
    la t1, excp_vect_table
    la t2, excp_vect_table_end
    move a0, sp /* pt_regs */
    add t0, t1, t0
    /* Check if exception code lies within bounds */
    bgeu t0, t2, 1f
    ld t0, 0(t0)
    jr t0
1:
    tail do_trap_unknown

handle_syscall:
     /* save the initial A0 value (needed in signal handlers) */
    sd a0, PT_ORIG_A0(sp)
    /*
     * Advance SEPC to avoid executing the original
     * scall instruction on sret
     */
    addi s2, s2, 0x4
    sd s2, PT_EPC(sp)
check_syscall_nr:
    csrw 0x0, a7
    /* Check to make sure we don't jump to a bogus syscall number. */
    li t0, __NR_syscalls
    la s0, sys_ni_syscall
    /*
     * Syscall number held in a7.
     * If syscall number is above allowed value, redirect to ni_syscall.
     */
    bge a7, t0, 1f
    /*
     * Check if syscall is rejected by tracer, i.e., a7 == -1.
     * If yes, we pretend it was executed.
     */
    li t1, -1
    beq a7, t1, ret_from_syscall_rejected
    blt a7, t1, 1f
    /* Call syscall */
    la s0, sys_call_table
    slli t0, a7, RISCV_LGPTR
    add s0, s0, t0
    ld s0, 0(s0)
1:
    jalr s0

ret_from_syscall:
    /* Set user a0 to kernel a0 */
    sd a0, PT_A0(sp)
    /*
     * We didn't execute the actual syscall.
     * Seccomp already set return value for the current task pt_regs.
     * (If it was configured with SECCOMP_RET_ERRNO/TRACE)
     */

ret_from_syscall_rejected:

ret_from_exception:
	ld s0, PT_STATUS(sp)
	csrc CSR_SSTATUS, SR_SIE
	andi s0, s0, SR_SPP
	bnez s0, restore_all

resume_userspace:
    /* Interrupts must be disabled here so flags are checked atomically */
    ld s0, TASK_TI_FLAGS(tp)    /* current_thread_info->flags */
    andi s1, s0, _TIF_WORK_MASK
    bnez s1, work_pending

    /* Save unwound kernel stack pointer in thread_info */
    addi s0, sp, PT_SIZE_ON_STACK
    sd s0, TASK_TI_KERNEL_SP(tp)

    /*
     * Save TP into the scratch register , so we can find the kernel data
     * structures again.
     */
    csrw CSR_SCRATCH, tp

restore_all:
	ld a0, PT_STATUS(sp)
	ld a2, PT_EPC(sp)

	csrw CSR_SSTATUS, a0
	csrw CSR_SEPC, a2

	ld x1,  PT_RA(sp)
	ld x3,  PT_GP(sp)
	ld x4,  PT_TP(sp)
	ld x5,  PT_T0(sp)
	ld x6,  PT_T1(sp)
	ld x7,  PT_T2(sp)
	ld x8,  PT_S0(sp)
	ld x9,  PT_S1(sp)
	ld x10, PT_A0(sp)
	ld x11, PT_A1(sp)
	ld x12, PT_A2(sp)
	ld x13, PT_A3(sp)
	ld x14, PT_A4(sp)
	ld x15, PT_A5(sp)
	ld x16, PT_A6(sp)
	ld x17, PT_A7(sp)
	ld x18, PT_S2(sp)
	ld x19, PT_S3(sp)
	ld x20, PT_S4(sp)
	ld x21, PT_S5(sp)
	ld x22, PT_S6(sp)
	ld x23, PT_S7(sp)
	ld x24, PT_S8(sp)
	ld x25, PT_S9(sp)
	ld x26, PT_S10(sp)
	ld x27, PT_S11(sp)
	ld x28, PT_T3(sp)
	ld x29, PT_T4(sp)
	ld x30, PT_T5(sp)
	ld x31, PT_T6(sp)
	ld x2,  PT_SP(sp)
	sret

work_pending:
    /* Todo: */
	sret

.align 2
.globl ret_from_kernel_thread
ret_from_kernel_thread:
    call schedule_tail
    /* Call fn(arg) */
    la ra, ret_from_exception
    move a0, s1
    jr s0

/*
 * Integer register context switch
 * The callee-saved registers must be saved and restored.
 *
 *   a0: previous task_struct (must be preserved across the switch)
 *   a1: next task_struct
 *
 * The value of a0 and a1 must be preserved by this function, as that's how
 * arguments are passed to schedule_tail.
 */
.align 2
.globl __switch_to
__switch_to:
    /* Save context into prev->thread */
    li    a4,  TASK_THREAD_RA
    add   a3, a0, a4
    add   a4, a1, a4
    sd ra,  TASK_THREAD_RA_RA(a3)
    sd sp,  TASK_THREAD_SP_RA(a3)
    sd s0,  TASK_THREAD_S0_RA(a3)
    sd s1,  TASK_THREAD_S1_RA(a3)
    sd s2,  TASK_THREAD_S2_RA(a3)
    sd s3,  TASK_THREAD_S3_RA(a3)
    sd s4,  TASK_THREAD_S4_RA(a3)
    sd s5,  TASK_THREAD_S5_RA(a3)
    sd s6,  TASK_THREAD_S6_RA(a3)
    sd s7,  TASK_THREAD_S7_RA(a3)
    sd s8,  TASK_THREAD_S8_RA(a3)
    sd s9,  TASK_THREAD_S9_RA(a3)
    sd s10, TASK_THREAD_S10_RA(a3)
    sd s11, TASK_THREAD_S11_RA(a3)
    /* Restore context from next->thread */
    ld ra,  TASK_THREAD_RA_RA(a4)
    ld sp,  TASK_THREAD_SP_RA(a4)
    ld s0,  TASK_THREAD_S0_RA(a4)
    ld s1,  TASK_THREAD_S1_RA(a4)
    ld s2,  TASK_THREAD_S2_RA(a4)
    ld s3,  TASK_THREAD_S3_RA(a4)
    ld s4,  TASK_THREAD_S4_RA(a4)
    ld s5,  TASK_THREAD_S5_RA(a4)
    ld s6,  TASK_THREAD_S6_RA(a4)
    ld s7,  TASK_THREAD_S7_RA(a4)
    ld s8,  TASK_THREAD_S8_RA(a4)
    ld s9,  TASK_THREAD_S9_RA(a4)
    ld s10, TASK_THREAD_S10_RA(a4)
    ld s11, TASK_THREAD_S11_RA(a4)
    /* Swap the CPU entry around. */
    lw a3, TASK_TI_CPU(a0)
    lw a4, TASK_TI_CPU(a1)
    sw a3, TASK_TI_CPU(a1)
    sw a4, TASK_TI_CPU(a0)
    /* The offset of thread_info in task_struct is zero. */
    move tp, a1
    ret

.section .rodata
/* Exception vector table */
.align 2
.globl excp_vect_table
excp_vect_table:
    RISCV_PTR do_trap_unknown
    RISCV_PTR do_trap_unknown
    RISCV_PTR do_trap_unknown
    RISCV_PTR do_trap_unknown
    RISCV_PTR do_trap_unknown
    RISCV_PTR do_trap_unknown
    RISCV_PTR do_trap_unknown
    RISCV_PTR do_trap_unknown
    RISCV_PTR do_trap_unknown   /* system call, gets intercepted */
    RISCV_PTR do_trap_unknown
    RISCV_PTR do_trap_unknown
    RISCV_PTR do_trap_unknown
    RISCV_PTR do_page_fault     /* instruction page fault */
    RISCV_PTR do_page_fault     /* load page fault */
    RISCV_PTR do_trap_unknown
    RISCV_PTR do_page_fault     /* store page fault */
excp_vect_table_end:
