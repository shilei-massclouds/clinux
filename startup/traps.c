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
