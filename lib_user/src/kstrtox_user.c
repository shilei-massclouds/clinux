// SPDX-License-Identifier: GPL-2.0
/*
 * Convert integer string representation to an integer.
 * If an integer doesn't fit into specified type, -E is returned.
 *
 * Integer starts with optional sign.
 * kstrtou*() functions do not accept sign "-".
 *
 * Radix 0 means autodetection: leading "0x" implies radix 16,
 * leading "0" implies radix 8, otherwise radix is 10.
 * Autodetection hints work after optional sign, but not before.
 *
 * If -E is returned, result is not touched.
 */
#include <linux/ctype.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/math64.h>
#include <linux/export.h>
#include <linux/types.h>
#include <linux/uaccess.h>
//#include "kstrtox.h"

/*
 * Since "base" would be a nonsense argument, this open-codes the
 * _from_user helper instead of using the helper macro below.
 */
int kstrtobool_from_user(const char __user *s, size_t count, bool *res)
{
	/* Longest string needed to differentiate, newline, terminator */
	char buf[4];

	count = min(count, sizeof(buf) - 1);
	if (copy_from_user(buf, s, count))
		return -EFAULT;
	buf[count] = '\0';
	return kstrtobool(buf, res);
}
EXPORT_SYMBOL(kstrtobool_from_user);

#define kstrto_from_user(f, g, type)					\
int f(const char __user *s, size_t count, unsigned int base, type *res)	\
{									\
	/* sign, base 2 representation, newline, terminator */		\
	char buf[1 + sizeof(type) * 8 + 1 + 1];				\
									\
	count = min(count, sizeof(buf) - 1);				\
	if (copy_from_user(buf, s, count))				\
		return -EFAULT;						\
	buf[count] = '\0';						\
	return g(buf, base, res);					\
}									\
EXPORT_SYMBOL(f)

kstrto_from_user(kstrtoull_from_user,	kstrtoull,	unsigned long long);
kstrto_from_user(kstrtoll_from_user,	kstrtoll,	long long);
kstrto_from_user(kstrtoul_from_user,	kstrtoul,	unsigned long);
kstrto_from_user(kstrtol_from_user,	kstrtol,	long);
kstrto_from_user(kstrtouint_from_user,	kstrtouint,	unsigned int);
kstrto_from_user(kstrtoint_from_user,	kstrtoint,	int);
kstrto_from_user(kstrtou16_from_user,	kstrtou16,	u16);
kstrto_from_user(kstrtos16_from_user,	kstrtos16,	s16);
kstrto_from_user(kstrtou8_from_user,	kstrtou8,	u8);
kstrto_from_user(kstrtos8_from_user,	kstrtos8,	s8);
