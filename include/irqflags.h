/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_TRACE_IRQFLAGS_H
#define _LINUX_TRACE_IRQFLAGS_H

#include <csr.h>

/* unconditionally enable interrupts */
static inline void arch_local_irq_enable(void)
{
    csr_set(CSR_STATUS, SR_IE);
}

/* unconditionally disable interrupts */
static inline void arch_local_irq_disable(void)
{
    csr_clear(CSR_STATUS, SR_IE);
}

#define raw_local_irq_enable()  arch_local_irq_enable()

#define local_irq_enable()  do { raw_local_irq_enable(); } while (0)

#endif /* _LINUX_TRACE_IRQFLAGS_H */
