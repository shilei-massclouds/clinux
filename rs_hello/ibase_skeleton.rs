// SPDX-License-Identifier: GPL-2.0

//! Component [rs_hello]

use kernel::prelude::*;
use kernel::interfaces::ibase::IBase;

#[doc(hidden)]
#[no_mangle]
extern "C" fn IBase_ready() -> bool {
    if let Some(m) = unsafe { &crate::__MOD } {
        return m.ready();
    }
    // Todo: Implement panic
    loop {}
}

#[no_mangle]
#[link_section = "_ksymtab_strings"]
static EXPORT_STR_IBase_ready: [u8; 11+1] = *b"IBase_ready\0";

#[no_mangle]
#[link_section = "_ksymtab"]
static EXPORT_SYM_IBase_ready: ExportSymbol = ExportSymbol {
    value: IBase_ready as *const fn(),
    name: EXPORT_STR_IBase_ready.as_ptr(),
};

#[doc(hidden)]
#[no_mangle]
extern "C" fn IBase_name() -> *const core::ffi::c_char {
    if let Some(m) = unsafe { &crate::__MOD } {
        return m.name();
    }
    // Todo: Implement panic
    loop {}
}

#[no_mangle]
#[link_section = "_ksymtab_strings"]
static EXPORT_STR_IBase_name: [u8; 10+1] = *b"IBase_name\0";

#[no_mangle]
#[link_section = "_ksymtab"]
static EXPORT_SYM_IBase_name: ExportSymbol = ExportSymbol {
    value: IBase_name as *const fn(),
    name: EXPORT_STR_IBase_name.as_ptr(),
};

