// SPDX-License-Identifier: GPL-2.0

use kernel::interfaces::ibase::IBase;
use kernel::interfaces::ilib::ILib;
use crate::ilib_stub::get_ilib;

pub(crate) struct Component {}

impl IBase for Component {
    fn ready(&self) -> bool {
        true
    }

    fn name(&self) -> *const core::ffi::c_char {
        let bitmap: u64 = 0;
        let itf_lib = get_ilib();
        let _ = itf_lib.find_next_bit(&bitmap as *const u64, 64, 0);
        "rust_hello\0".as_ptr() as *const core::ffi::c_char
    }
}

