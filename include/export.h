/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef _LINUX_EXPORT_H
#define _LINUX_EXPORT_H

#include <compiler_attributes.h>

#define _KSYMTAB_ENTRY(sym)                               \
    static const struct kernel_symbol _ksymtab_##sym           \
    __attribute__((section("_ksymtab"), used))   \
    __aligned(sizeof(void *))                                   \
    = { (unsigned long)&sym, _kstrtab_##sym }

struct kernel_symbol {
    unsigned long value;
    const char *name;
};

#define EXPORT_SYMBOL(sym)                                          \
    extern typeof(sym) sym;                                         \
    extern const char _kstrtab_##sym[];                            \
    asm("   .section \"_ksymtab_strings\",\"aMS\",%progbits,1  \n" \
        "_kstrtab_" #sym ":                    \n"                 \
        "   .asciz  \"" #sym "\"                \n"                 \
        "   .previous                           \n");               \
    _KSYMTAB_ENTRY(sym)

#endif /* _LINUX_EXPORT_H */
