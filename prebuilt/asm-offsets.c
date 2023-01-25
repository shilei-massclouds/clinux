// SPDX-License-Identifier: GPL-2.0-only

#include <sched.h>
#include <kbuild.h>

void asm_offsets(void)
{
    OFFSET(TASK_THREAD_RA, task_struct, thread.ra);

    DEFINE(TASK_THREAD_RA_RA,
          offsetof(struct task_struct, thread.ra)
        - offsetof(struct task_struct, thread.ra)
    );
    DEFINE(TASK_THREAD_SP_RA,
          offsetof(struct task_struct, thread.sp)
        - offsetof(struct task_struct, thread.ra)
    );
    DEFINE(TASK_THREAD_S0_RA,
          offsetof(struct task_struct, thread.s[0])
        - offsetof(struct task_struct, thread.ra)
    );
    DEFINE(TASK_THREAD_S1_RA,
          offsetof(struct task_struct, thread.s[1])
        - offsetof(struct task_struct, thread.ra)
    );
    DEFINE(TASK_THREAD_S2_RA,
          offsetof(struct task_struct, thread.s[2])
        - offsetof(struct task_struct, thread.ra)
    );
    DEFINE(TASK_THREAD_S3_RA,
          offsetof(struct task_struct, thread.s[3])
        - offsetof(struct task_struct, thread.ra)
    );
    DEFINE(TASK_THREAD_S4_RA,
          offsetof(struct task_struct, thread.s[4])
        - offsetof(struct task_struct, thread.ra)
    );
    DEFINE(TASK_THREAD_S5_RA,
          offsetof(struct task_struct, thread.s[5])
        - offsetof(struct task_struct, thread.ra)
    );
    DEFINE(TASK_THREAD_S6_RA,
          offsetof(struct task_struct, thread.s[6])
        - offsetof(struct task_struct, thread.ra)
    );
    DEFINE(TASK_THREAD_S7_RA,
          offsetof(struct task_struct, thread.s[7])
        - offsetof(struct task_struct, thread.ra)
    );
    DEFINE(TASK_THREAD_S8_RA,
          offsetof(struct task_struct, thread.s[8])
        - offsetof(struct task_struct, thread.ra)
    );
    DEFINE(TASK_THREAD_S9_RA,
          offsetof(struct task_struct, thread.s[9])
        - offsetof(struct task_struct, thread.ra)
    );
    DEFINE(TASK_THREAD_S10_RA,
          offsetof(struct task_struct, thread.s[10])
        - offsetof(struct task_struct, thread.ra)
    );
    DEFINE(TASK_THREAD_S11_RA,
          offsetof(struct task_struct, thread.s[11])
        - offsetof(struct task_struct, thread.ra)
    );
}
