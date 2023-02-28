// SPDX-License-Identifier: GPL-2.0

//! Component [rs_hello].

use core::{
    ffi::{c_char, c_void},
    fmt,
};
use kernel::prelude::*;

module! {
    type: FrameWork,
    name: "rust",
    author: "Rust for Linux Contributors",
    description: "Rust hello_world sample",
    license: "GPL",
}

struct FrameWork;

impl kernel::Module for FrameWork {
    fn init(_name: &'static CStr, _module: &'static ThisModule) -> Result<Self> {
        Ok(FrameWork { })
    }
}

impl Drop for FrameWork {
    fn drop(&mut self) {
    }
}

pub const FMT_LENGTH: usize = 10;

extern "C" {
    pub fn printk(fmt: *const core::ffi::c_char, ...) -> core::ffi::c_int;
}

#[doc(hidden)]
#[no_mangle]
pub unsafe fn call_printk(
    format_string: &[u8; FMT_LENGTH],
    module_name: &[u8],
    args: fmt::Arguments<'_>,
) {
    // `_printk` does not seem to fail in any path.
    unsafe {
        printk(
            format_string.as_ptr() as _,
            module_name.as_ptr(),
            &args as *const _ as *const c_void,
        );
    }
}

export_symbol! {
    symbol: call_printk,
}
