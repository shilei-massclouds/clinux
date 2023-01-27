/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef _ASM_RISCV_BUG_H
#define _ASM_RISCV_BUG_H

#include <sbi.h>
#include <printk.h>

#define halt sbi_srst_power_off

#define BUG() do {    \
    printk("\n########################\n"); \
    printk("BUG: %s (%s:%u)", __FUNCTION__, __FILE__, __LINE__); \
    printk("\n########################\n"); \
    halt(); \
} while (0)

#define BUG_ON(cond) do {   \
    if ((cond)) BUG();      \
} while (0)

#define panic(args...) \
    do { \
        printk("\n########################\n"); \
        printk("PANIC: %s (%s:%u)\n", __FUNCTION__, __FILE__, __LINE__); \
        printk(args); \
        printk("\n########################\n"); \
        halt(); \
    } while(0)

#endif /* _ASM_RISCV_BUG_H */
