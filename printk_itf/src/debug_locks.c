// SPDX-License-Identifier: GPL-2.0-only

#include <linux/debug_locks.h>

/*
 * We want to turn all lock-debugging facilities on/off at once,
 * via a global flag. The reason is that once a single bug has been
 * detected and reported, there might be cascade of followup bugs
 * that would just muddy the log. So we report the first one and
 * shut up after that.
 */
int debug_locks __read_mostly = 1;
EXPORT_SYMBOL_GPL(debug_locks);

/*
 * The locking-testsuite uses <debug_locks_silent> to get a
 * 'silent failure': nothing is printed to the console when
 * a locking bug is detected.
 */
int debug_locks_silent __read_mostly;
EXPORT_SYMBOL_GPL(debug_locks_silent);
