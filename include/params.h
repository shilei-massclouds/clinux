/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_PARAMS_H
#define _LINUX_PARAMS_H

struct kernel_param {
    const char *name;
    int (*setup_func)(char *param, char *value);
};

int
parse_args(char *args,
           const struct kernel_param *params,
           unsigned int num);

#endif /* _LINUX_PARAMS_H */
