OUTPUT_ARCH(riscv)
ENTRY(_start)
SECTIONS
{
    . = 0xffffffe000000000;
    _start = .;
    .head.text : AT(ADDR(.head.text) - 0xffffffe000000000) {
        *(.head.text);
    }
    . = ALIGN((1 << 12));
    .init.text : AT(ADDR(.init.text) - 0xffffffe000000000) {
        *(.init.text);
    }
    .init.data : AT(ADDR(.init.data) - 0xffffffe000000000) {
        *(.init.data);
    }
    _text_start = .;
    .text : AT(ADDR(.text) - 0xffffffe000000000) {
        *(.text);
    }
    _text_end = .;
    _rodata_start = .;
    .rodata : AT(ADDR(.rodata) - 0xffffffe000000000) {
        *(.rodata);
    }
    _rodata_end = .;
    _ksymtab : AT(ADDR(_ksymtab) - 0xffffffe000000000) {
        _start_ksymtab = .;
        KEEP(*(SORT(_ksymtab)));
        _end_ksymtab = .;
    }
    _ksymtab_strings : AT(ADDR(_ksymtab_strings) - 0xffffffe000000000) {
        _start_ksymtab_strings = .;
        KEEP(*(_ksymtab_strings));
        _end_ksymtab_strings = .;
    }
    _data_start = .;
    sdata = .;
    .data : AT(ADDR(.data) - 0xffffffe000000000) {
        . = ALIGN((1 << 12));
        _init_stack = .;
        init_stack = .;
        . = init_stack + ((1 << 12) << 2);
        _init_stack_top = .;
        init_stack_top = .;
        *(.data);
        *(.sdata);
        __global_pointer$ = . + 0x800;
    }
    edata = .;
    _data_end = .;
    . = ALIGN((1 << 12));
    _bss_start = .;
    . = ALIGN((1 << 12));
    .sbss : AT(ADDR(.sbss) - 0xffffffe000000000) {
        *(.sbss);
    }
    . = ALIGN((1 << 12));
    .bss : AT(ADDR(.bss) - 0xffffffe000000000) {
        . = ALIGN((1 << 12));
        *(.bss..page_aligned);
        . = ALIGN((1 << 12));
        *(.bss);
    }
    _bss_stop = .;
    ekernel = .;
    _end = .;
}
