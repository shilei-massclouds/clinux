// SPDX-License-Identifier: GPL-2.0

use kernel::prelude::*;
use kernel::print::printk;
use crate::IBase;

pub(crate) struct Component {}

impl IBase for Component {
    fn ready(&self) -> bool {
        true
    }

    fn name(&self) -> *const core::ffi::c_char {
        "rust_hello\0".as_ptr() as *const core::ffi::c_char
    }
}
