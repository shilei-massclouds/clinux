// SPDX-License-Identifier: GPL-2.0-only

extern void say_hello();

int
init_module(void)
{
    //printk("module[helloworld]: init begin ...\n");
    say_hello();
    //printk("module[helloworld]: init end!\n");
    return 0;
}
