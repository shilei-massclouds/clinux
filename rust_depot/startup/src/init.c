/* SPDX-License-Identifier: GPL-2.0-only */

#include <sbi.h>
#include <export.h>

extern char skernel[];
EXPORT_SYMBOL(skernel);
extern char ekernel[];
EXPORT_SYMBOL(ekernel);

extern char sdata[];
EXPORT_SYMBOL(sdata);
extern char edata[];
EXPORT_SYMBOL(edata);

extern char sbss[];
EXPORT_SYMBOL(sbss);
extern char ebss[];
EXPORT_SYMBOL(ebss);

extern char stext[];
EXPORT_SYMBOL(stext);
extern char etext[];
EXPORT_SYMBOL(etext);

extern char srodata[];
EXPORT_SYMBOL(srodata);
extern char erodata[];
EXPORT_SYMBOL(erodata);

extern char boot_stack[];
EXPORT_SYMBOL(boot_stack);
extern char boot_stack_top[];
EXPORT_SYMBOL(boot_stack_top);

void unreachable(void)
{
    sbi_puts("\n##########################");
    sbi_puts("\nImpossible to come here!\n");
    sbi_puts("##########################\n");

    sbi_srst_power_off();
}
