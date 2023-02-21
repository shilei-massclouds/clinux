// SPDX-License-Identifier: GPL-2.0

//! Rust minimal sample.

use kernel::prelude::*;
use kernel::print::printk;

/***************************/
/* substitue for EXPORT_SYMBOL */

/*
#[no_mangle]
#[link_section = "_ksymtab_strings"]
static EXPORT_STR: [u8; 6] = *b"ready\0";

#[no_mangle]
#[link_section = "_ksymtab"]
static EXPORT_SYM: ExportSymbol = ExportSymbol {
    value: RustHello::ready as *const fn(),
    name: EXPORT_STR.as_ptr(),
};
*/

/***************************/

trait IBase {
    fn ready() -> bool;
}

provide! {
    interface: IBase,
    component: RustHello,
}

module! {
    type: RustHello,
    name: "rust_hello",
    author: "Rust for Linux Contributors",
    description: "Rust hello_world sample",
    license: "GPL",
}

struct RustHello {}

impl IBase for RustHello {
    #[no_mangle]
    fn ready() -> bool {
        true
    }
}

impl kernel::Module for RustHello {
    fn init(_name: &'static CStr, _module: &'static ThisModule) -> Result<Self> {
        unsafe {
            printk(b"Rust: Hello world!\n\0");
        }

        Ok(RustHello { })
    }
}

impl Drop for RustHello {
    fn drop(&mut self) {
    }
}

