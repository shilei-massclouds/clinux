// SPDX-License-Identifier: GPL-2.0-only

#ifndef _CL_TYPES_H_
#define _CL_TYPES_H_

#include <linux/init.h>

extern void parse_dtb(void);

extern int do_early_param(char *param, char *val,
                 const char *unused, void *arg);

extern void setup_kernel_in_mm(void);
extern void setup_bootmem(void);

#endif /* _CL_TYPES_H_ */
