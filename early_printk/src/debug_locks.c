// SPDX-License-Identifier: GPL-2.0-only
/*
 * lib/debug_locks.c
 *
 * Generic place for common debugging facilities for various locks:
 * spinlocks, rwlocks, mutexes and rwsems.
 *
 * Started by Ingo Molnar:
 *
 *  Copyright (C) 2006 Red Hat, Inc., Ingo Molnar <mingo@redhat.com>
 */
#include <linux/rwsem.h>
#include <linux/mutex.h>
#include <linux/export.h>
#include <linux/spinlock.h>
#include <linux/debug_locks.h>

/*
 * Generic 'turn off all lock debugging' function:
 */
noinstr int debug_locks_off(void)
{
	if (debug_locks && __debug_locks_off()) {
		if (!debug_locks_silent) {
			console_verbose();
			return 1;
		}
	}
	return 0;
}
