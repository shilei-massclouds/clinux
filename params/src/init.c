// SPDX-License-Identifier: GPL-2.0-only

#include <linux/init.h>
#include <linux/types.h>
#include <linux/export.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <linux/moduleparam.h>
#include <asm/setup.h>
#include <cl_hook.h>
#include "../../booter/src/booter.h"

extern const struct obs_kernel_param __setup_start[], __setup_end[];

int
cl_params_init(void)
{
    sbi_puts("module[params]: init begin ...\n");
    parse_early_param();
    sbi_puts("module[params]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_params_init);

DEFINE_ENABLE_FUNC(params);

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

void __init parse_early_options(char *cmdline)
{
    parse_args("early options", cmdline, NULL, 0, 0, 0, NULL,
           do_early_param);
}

/* Arch code calls this early on, or if not, just before other parsing. */
void __init parse_early_param(void)
{
    static int done __initdata;
    static char tmp_cmdline[COMMAND_LINE_SIZE] __initdata;

    if (done)
        return;

    /* All fall through to do_early_param. */
    strlcpy(tmp_cmdline, boot_command_line, COMMAND_LINE_SIZE);
    parse_early_options(tmp_cmdline);
    done = 1;
}
