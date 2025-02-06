// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/kernel.h>
#include <linux/mm_types.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"
#include "sched.h"

/* arch/riscv/kernel/cpufeature.c */
bool has_fpu __read_mostly;

/* kernel/panic.c */
int panic_on_warn __read_mostly;

enum system_states system_state __read_mostly;
EXPORT_SYMBOL(system_state);

int
cl_sched_init(void)
{
    sbi_puts("module[sched]: init begin ...\n");
    REQUIRE_COMPONENT(task);
    sbi_puts("module[sched]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_sched_init);

u64 sched_clock_cpu(int cpu)
{
    booter_panic("in sched!");
}

void switch_mm(struct mm_struct *prev, struct mm_struct *next,
    struct task_struct *task)
{
    booter_panic("in sched!");
}

void put_task_struct_rcu_user(struct task_struct *task)
{
    booter_panic("in sched!");
}

void print_modules(void)
{
    booter_panic("in sched!");
}

extern struct task_struct *__switch_to(struct task_struct *,
                       struct task_struct *)
{
    booter_panic("in sched!");
}

void put_task_stack(struct task_struct *tsk)
{
    booter_panic("in sched!");
}

struct task_struct *pick_next_task_idle(struct rq *rq)
{
    booter_panic("in sched!");
}

void __mmdrop(struct mm_struct *mm)
{
    booter_panic("in sched!");
}

void rcu_qs(void)
{
    booter_panic("in sched!");
}

void panic(const char *fmt, ...)
{
    booter_panic("No impl 'panic'.");
    do {} while(1);
}
EXPORT_SYMBOL(panic);
