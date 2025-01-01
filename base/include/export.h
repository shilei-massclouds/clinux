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

#define _KSYMTAB_ENTRY_ITF(sym, itf)                          \
    static const struct kernel_symbol _ksymtab_##itf##_##sym    \
    __attribute__((section("_ksymtab"), used))   \
    __aligned(sizeof(void *))                                   \
    = { (unsigned long)&sym, _kstrtab_##itf##_##sym }

#define EXPORT_SYMBOL_ITF(sym, itf)                                          \
    extern typeof(sym) sym;                                         \
    extern const char _kstrtab_##itf##_##sym[];                     \
    asm("   .section \"_ksymtab_strings\",\"aMS\",%progbits,1  \n" \
        "_kstrtab_" #itf "_" #sym ":            \n"                 \
        "   .asciz  \"" #itf "_" #sym "\"       \n"                 \
        "   .previous                           \n");               \
    _KSYMTAB_ENTRY_ITF(sym, itf)

extern struct module __this_module;
#define THIS_MODULE (&__this_module)

#define EXPORT_SYMBOL_RUST_GPL(sym) extern int sym; EXPORT_SYMBOL(sym)

#endif /* _LINUX_EXPORT_H */
