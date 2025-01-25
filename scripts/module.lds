/* SPDX-License-Identifier: GPL-2.0-only */

SECTIONS
{
    __ksymtab 0 : {
        __start_mod_ksymtab = .;
        KEEP(*(SORT(___ksymtab+*)));
        __end_mod_ksymtab = .;
    }
    __ksymtab_strings 0 : { KEEP(*(__ksymtab_strings)); }
}
