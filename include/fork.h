/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_FORK_H
#define _LINUX_FORK_H

#include <sched.h>
#include <mm_types.h>

struct mm_struct *mm_alloc(void);

typedef struct vm_area_struct *
(*vm_area_alloc_t)(struct mm_struct *mm);
extern vm_area_alloc_t vm_area_alloc;

typedef void
(*vm_area_free_t)(struct vm_area_struct *vma);
extern vm_area_free_t vm_area_free;

typedef struct vm_area_struct *
(*vm_area_dup_t)(struct vm_area_struct *orig);
extern vm_area_dup_t vm_area_dup;

void set_mm_exe_file(struct mm_struct *mm, struct file *new_exe_file);

pid_t kernel_thread(int (*fn)(void *), void *arg, unsigned long flags);

int copy_thread(unsigned long clone_flags,
                unsigned long usp, unsigned long arg,
                struct task_struct *p, unsigned long tls);

int arch_dup_task_struct(struct task_struct *dst, struct task_struct *src);

#endif /* _LINUX_FORK_H */
