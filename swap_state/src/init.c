// SPDX-License-Identifier: GPL-2.0-only

#include <linux/types.h>
#include <linux/export.h>
#include <linux/swap.h>
#include "../../booter/src/booter.h"

int
cl_swap_state_init(void)
{
    sbi_puts("module[swap_state]: init begin ...\n");
    sbi_puts("module[swap_state]: init end!\n");
    return 0;
}
EXPORT_SYMBOL(cl_swap_state_init);

/*
struct swap_info_struct *get_swap_device(swp_entry_t entry)
{
    booter_panic("No impl!\n");
}
struct swap_info_struct *swp_swap_info(swp_entry_t entry)
{
    booter_panic("No impl!\n");
}
*/

