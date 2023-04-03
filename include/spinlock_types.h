/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_SPINLOCK_TYPES_H
#define __LINUX_SPINLOCK_TYPES_H

typedef struct {
    volatile unsigned int lock;
} arch_spinlock_t;

#define __ARCH_SPIN_LOCK_UNLOCKED   { 0 }

typedef struct {
    volatile unsigned int lock;
} arch_rwlock_t;

#define __ARCH_RW_LOCK_UNLOCKED     { 0 }

typedef struct raw_spinlock {
    arch_spinlock_t raw_lock;
} raw_spinlock_t;

typedef struct spinlock {
    struct raw_spinlock rlock;
} spinlock_t;

#endif /* __LINUX_SPINLOCK_TYPES_H */
