/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _LINUX_HASH_TABLE_H_
#define _LINUX_HASH_TABLE_H_

#include <list.h>

#define HASH_ZERO   0x00000004  /* Zero allocated hash table */

void *
alloc_large_system_hash(const char *tablename,
                        unsigned long bucketsize,
                        int scale,
                        unsigned int *_hash_shift,
                        unsigned int *_hash_mask);

#endif /* _LINUX_HASH_TABLE_H_ */
