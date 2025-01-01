/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LIMITS_H
#define __LIMITS_H

#define ARG_MAX     131072  /* # bytes of args + environ for exec() */

#define ULONG_MAX   (~0UL)
#define PATH_MAX    4096    /* # chars in a path name including nul */
#define NAME_MAX    255     /* # chars in a file name */
#define UINT_MAX    (~0U)
#define LONG_MAX    ((long)(~0UL >> 1))
#define LLONG_MAX   ((long long)(~0ULL >> 1))

#define CONFIG_INIT_ENV_ARG_LIMIT 32

#endif /* __LIMITS_H */
