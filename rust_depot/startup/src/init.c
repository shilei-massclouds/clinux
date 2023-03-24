/* SPDX-License-Identifier: GPL-2.0-only */

#include <sbi.h>
#include <export.h>

extern char ekernel[];
EXPORT_SYMBOL(ekernel);

extern char sdata[];
EXPORT_SYMBOL(sdata);

extern char edata[];
EXPORT_SYMBOL(edata);

extern char sbss[];
EXPORT_SYMBOL(sbss);

extern char stext[];
EXPORT_SYMBOL(stext);

void unreachable(void)
{
    sbi_puts("\n##########################");
    sbi_puts("\nImpossible to come here!\n");
    sbi_puts("##########################\n");

    sbi_srst_power_off();
}
