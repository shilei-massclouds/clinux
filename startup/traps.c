// SPDX-License-Identifier: GPL-2.0-only

#include <sbi.h>
#include <ptrace.h>
#include <signal.h>

static void
do_trap_error(struct pt_regs *regs, int signo, int code,
              unsigned long addr, const char *str)
{
    sbi_puts(str);
    sbi_puts("\n");
    sbi_srst_power_off();
}

#define DO_ERROR_INFO(name, signo, code, str)   \
void name(struct pt_regs *regs)                 \
{                                               \
    do_trap_error(regs, signo, code, regs->epc, "Oops - " str); \
}

DO_ERROR_INFO(do_trap_unknown, SIGILL, ILL_ILLTRP, "unknown exception");
DO_ERROR_INFO(do_trap_insn_misaligned, SIGBUS, BUS_ADRALN,
              "instruction address misaligned");
DO_ERROR_INFO(do_trap_insn_fault, SIGSEGV, SEGV_ACCERR,
              "instruction access fault");
DO_ERROR_INFO(do_trap_insn_illegal, SIGILL, ILL_ILLOPC, "illegal instruction");
DO_ERROR_INFO(do_trap_load_fault, SIGSEGV, SEGV_ACCERR, "load access fault");
DO_ERROR_INFO(do_trap_load_misaligned, SIGBUS, BUS_ADRALN,
              "Oops - load address misaligned");
DO_ERROR_INFO(do_trap_store_misaligned, SIGBUS, BUS_ADRALN,
              "Oops - store (or AMO) address misaligned");
DO_ERROR_INFO(do_trap_store_fault, SIGSEGV, SEGV_ACCERR,
              "store (or AMO) access fault");
DO_ERROR_INFO(do_trap_ecall_u, SIGILL, ILL_ILLTRP,
              "environment call from U-mode");
DO_ERROR_INFO(do_trap_ecall_s, SIGILL, ILL_ILLTRP,
              "environment call from S-mode");
DO_ERROR_INFO(do_trap_ecall_m, SIGILL, ILL_ILLTRP,
              "environment call from M-mode");
