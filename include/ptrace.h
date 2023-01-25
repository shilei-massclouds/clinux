/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef _ASM_RISCV_PTRACE_H
#define _ASM_RISCV_PTRACE_H

/* ALIGN(sizeof(struct pt_regs), STACK_ALIGN) */
#define PT_SIZE_ON_STACK 288
#define PT_SIZE 288 /* sizeof(struct pt_regs) */

#define PT_EPC 0

#define PT_RA 8
#define PT_SP 16
#define PT_GP 24
#define PT_TP 32
#define PT_T0 40
#define PT_T1 48
#define PT_T2 56
#define PT_S0 64
#define PT_S1 72
#define PT_A0 80
#define PT_A1 88
#define PT_A2 96
#define PT_A3 104
#define PT_A4 112
#define PT_A5 120
#define PT_A6 128
#define PT_A7 136
#define PT_S2 144
#define PT_S3 152
#define PT_S4 160
#define PT_S5 168
#define PT_S6 176
#define PT_S7 184
#define PT_S8 192
#define PT_S9 200
#define PT_S10 208
#define PT_S11 216
#define PT_T3 224
#define PT_T4 232
#define PT_T5 240
#define PT_T6 248
#define PT_STATUS  256
#define PT_BADADDR 264
#define PT_CAUSE   272
#define PT_ORIG_A0 280

#define PT_FP 64

#ifndef __ASSEMBLY__

struct pt_regs {
    unsigned long epc;
    unsigned long ra;
    unsigned long sp;
    unsigned long gp;
    unsigned long tp;
    unsigned long t0;
    unsigned long t1;
    unsigned long t2;
    unsigned long s0;
    unsigned long s1;
    unsigned long a0;
    unsigned long a1;
    unsigned long a2;
    unsigned long a3;
    unsigned long a4;
    unsigned long a5;
    unsigned long a6;
    unsigned long a7;
    unsigned long s2;
    unsigned long s3;
    unsigned long s4;
    unsigned long s5;
    unsigned long s6;
    unsigned long s7;
    unsigned long s8;
    unsigned long s9;
    unsigned long s10;
    unsigned long s11;
    unsigned long t3;
    unsigned long t4;
    unsigned long t5;
    unsigned long t6;
    /* Supervisor/Machine CSRs */
    unsigned long status;
    unsigned long badaddr;
    unsigned long cause;
    /* a0 value before the syscall */
    unsigned long orig_a0;
};

#define current_pt_regs() task_pt_regs(current)
#define user_mode(regs) (((regs)->status & SR_PP) == 0)

#endif /* __ASSEMBLY__ */

#endif /* _ASM_RISCV_PTRACE_H */
