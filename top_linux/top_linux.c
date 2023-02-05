// SPDX-License-Identifier: GPL-2.0-only

#include <mm.h>
#include <fs.h>
#include <bug.h>
#include <fdt.h>
#include <printk.h>
#include <platform.h>

static int
init_module(void)
{
    printk("module[top_linux]: init begin ...\n");

    printk("module[top_linux]: init end!\n");

    return 0;
}
