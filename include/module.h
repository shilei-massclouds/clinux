/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef _LINUX_MODULE_H
#define _LINUX_MODULE_H

#include <export.h>
#include <list.h>

typedef int (*init_module_t)(void);
typedef void (*exit_module_t)(void);

struct module {
    struct list_head list;

    const struct kernel_symbol *syms;
    unsigned int num_syms;

    /* Startup function. */
    init_module_t init;

    /* Destruction function. */
    exit_module_t exit;
};

#endif /* _LINUX_MODULE_H */
