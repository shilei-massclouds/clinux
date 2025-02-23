// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/file.h>
#include <linux/fs.h>
#include "../../booter/src/booter.h"

int
cl_file_init(void)
{
    sbi_puts("module[file]: init begin ...\n");
    sbi_puts("module[file]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_file_init);

/*
void __receive_sock(struct file *file)
{
    booter_panic("No impl.\n");
}
*/

