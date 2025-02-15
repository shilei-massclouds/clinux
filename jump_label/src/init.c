// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/cache.h>
#include "../../booter/src/booter.h"

int
cl_jump_label_init(void)
{
    sbi_puts("module[jump_label]: init begin ...\n");
    sbi_puts("module[jump_label]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_jump_label_init);

/*
 * Used to generate warnings if static_key manipulation functions are used
 * before jump_label_init is called.
 */
bool static_key_initialized __read_mostly;
EXPORT_SYMBOL_GPL(static_key_initialized);

/*
struct module *__module_address(unsigned long addr)
{
    booter_panic("No impl 'jump_label'.");
}

int patch_text_nosync(void *addr, const void *insns, size_t len)
{
    booter_panic("No impl 'jump_label'.");
}
*/
