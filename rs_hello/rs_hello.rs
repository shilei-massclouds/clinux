// SPDX-License-Identifier: GPL-2.0

//! Rust minimal sample.

use kernel::prelude::*;
use kernel::print::printk;

/***************************/
/* substitue for EXPORT_SYMBOL */

#[no_mangle]
#[link_section = "_ksymtab_strings"]
static EXPORT_STR: [u8; 6] = [b'r', b'e', b'a', b'd', b'y', 0];

struct kernel_symbol {
    value: *const fn(),
    name: *const u8,
}

unsafe impl Sync for kernel_symbol {}

#[no_mangle]
#[link_section = "_ksymtab"]
static EXPORT_SYM: kernel_symbol = kernel_symbol {
    value: RustHello::ready as *const fn(),
    name: &EXPORT_STR as *const u8,
};

/***************************/

trait IBase {
    fn ready() -> bool;
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

