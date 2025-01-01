// SPDX-License-Identifier: GPL-2.0

#include <mm.h>
#include <csr.h>
#include <sched.h>
#include <export.h>

void switch_mm(struct mm_struct *prev, struct mm_struct *next,
               struct task_struct *task)
{
    if (unlikely(prev == next))
        return;

    csr_write(CSR_SATP, virt_to_pfn(next->pgd) | SATP_MODE);
}
EXPORT_SYMBOL(switch_mm);
