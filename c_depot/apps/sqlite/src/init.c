// SPDX-License-Identifier: GPL-2.0-only

//#include <bug.h>
//#include <export.h>
//#include <printk.h>

//bool sqlite_ready = false;
//EXPORT_SYMBOL(sqlite_ready);

int
init_module(void)
{
    //printk("module[sqlite]: init begin ...\n");

    //sqlite_ready = true;

    //printk("module[sqlite]: init end!\n");
    return 0;
}
