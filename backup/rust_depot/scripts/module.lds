/* SPDX-License-Identifier: GPL-2.0-only */

SECTIONS
{
    _ksymtab 0 : {
        _start_mod_ksymtab = .;
        KEEP(*(SORT(_ksymtab)));
        _end_mod_ksymtab = .;
    }
    _ksymtab_strings 0 : { KEEP(*(_ksymtab_strings)); }
}
