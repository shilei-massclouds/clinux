// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include "../../booter/src/booter.h"

extern const struct obs_kernel_param __setup_start[], __setup_end[];

int
cl_params_init(void)
{
    sbi_puts("module[params]: init begin ...\n");
    sbi_puts("module[params]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_params_init);

/* Check for early params. */
int __init do_early_param(char *param, char *val,
                 const char *unused, void *arg)
{
    const struct obs_kernel_param *p;

    for (p = __setup_start; p < __setup_end; p++) {
        if ((p->early && parameq(param, p->str)) ||
            (strcmp(param, "console") == 0 &&
             strcmp(p->str, "earlycon") == 0)
        ) {
            if (p->setup_func(val) != 0)
                pr_warn("Malformed early option '%s'\n", param);
        }
    }
    /* We accept everything at this stage. */
    return 0;
}
EXPORT_SYMBOL(do_early_param);
