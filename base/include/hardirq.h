/* SPDX-License-Identifier: GPL-2.0 */
#ifndef LINUX_HARDIRQ_H
#define LINUX_HARDIRQ_H

#include <interrupt.h>

typedef struct {
    unsigned int __softirq_pending;
} irq_cpustat_t;

/*
 * Enter irq context (on NO_HZ, update jiffies):
 */
void irq_enter(void);

/*
 * Exit irq context and process softirqs if needed:
 */
void irq_exit(void);

void open_softirq(int nr, void (*action)(struct softirq_action *));

#endif /* LINUX_HARDIRQ_H */
