/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef _LINUX_MODULE_H
#define _LINUX_MODULE_H

#include <export.h>
#include <list.h>

/* Chosen so that structs with an unsigned long line up. */
#define MAX_PARAM_PREFIX_LEN (64 - sizeof(unsigned long))
#define MODULE_NAME_LEN MAX_PARAM_PREFIX_LEN

typedef int (*init_module_t)(void);
typedef void (*exit_module_t)(void);

/* These are either module local, or the kernel's dummy ones. */
extern int init_module(void);
extern void cleanup_module(void);

struct module {
    struct list_head list;

    /* Unique handle for this module */
    char name[MODULE_NAME_LEN];

    const struct kernel_symbol *syms;
    unsigned int num_syms;

    /* Startup function. */
    init_module_t init;

    /* Destruction function. */
    exit_module_t exit;
};

#endif /* _LINUX_MODULE_H */
