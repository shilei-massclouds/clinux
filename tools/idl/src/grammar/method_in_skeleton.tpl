#[doc(hidden)]
#[no_mangle]
pub extern "C" fn {{c_method}}({{decl_args}}){{rtype}} {
    if let Some(m) = unsafe { &__MOD } {
        return m.{{method}}({{call_args}});
    }
    // Todo: Implement panic
    loop {}
}

#[no_mangle]
#[link_section = "_ksymtab_strings"]
static EXPORT_STR_{{c_method}}: [u8; {{len}}+1] = *b"{{c_method}}\0";

#[no_mangle]
#[link_section = "_ksymtab"]
static EXPORT_SYM_{{c_method}}: ExportSymbol = ExportSymbol {
    value: {{c_method}} as *const fn(),
    name: EXPORT_STR_{{c_method}}.as_ptr(),
};
