// SPDX-License-Identifier: GPL-2.0-only

#include <bug.h>
#include <printk.h>

extern bool libax_ready(void);
extern void start_echo_server(void);

int
init_module(void)
{
    printk("module[top_echoserver]: init begin ...\n");
    BUG_ON(!libax_ready());
    start_echo_server();
    printk("module[top_echoserver]: init end!\n");
    return 0;
}
