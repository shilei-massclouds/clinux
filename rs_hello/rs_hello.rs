// SPDX-License-Identifier: GPL-2.0

//! Component [rs_hello]

use kernel::prelude::*;
use kernel::interfaces::ibase::IBase;
use crate::component::Component;

mod ilib_stub;
mod component;

#[doc(hidden)]
#[no_mangle]
pub extern "C" fn IBase_ready() -> bool {
    if let Some(m) = unsafe { &__MOD } {
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
pub extern "C" fn IBase_name() -> *const core::ffi::c_char {
    if let Some(m) = unsafe { &__MOD } {
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



module! {
    type: FrameWork,
    name: "rs_hello",
    author: "Rust for Linux Contributors",
    description: "Component rs_hello",
    license: "GPL",
}

struct FrameWork {
    com: Component,
}

impl IBase for FrameWork {
    fn ready(&self) -> bool {
        self.com.ready()
    }

    fn name(&self) -> *const core::ffi::c_char {
        self.com.name()
    }
}

impl kernel::Module for FrameWork {
    fn init(_name: &'static CStr, _module: &'static ThisModule) -> Result<Self> {
        pr_info!("[rs_hello]: init begin...\n");
        pr_info!("[rs_hello]: init end!\n");

        Ok(FrameWork { com: Component {} })
    }
}

impl Drop for FrameWork {
    fn drop(&mut self) {
    }
}
