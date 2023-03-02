// SPDX-License-Identifier: GPL-2.0

//! Component [rs_hello]

use kernel::prelude::*;
use kernel::interfaces::ibase::IBase;
use kernel::interfaces::ilib::ILib;
use crate::ilib_stub::get_ilib;

mod ilib_stub;
mod ibase_skeleton;

module! {
    type: FrameWork,
    name: "rs_hello",
    author: "Rust for Linux Contributors",
    description: "Component rs_hello",
    license: "GPL",
}

struct FrameWork;

impl IBase for FrameWork {
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

impl kernel::Module for FrameWork {
    fn init(_name: &'static CStr, _module: &'static ThisModule) -> Result<Self> {
        pr_info!("[rs_hello]: init begin...\n");
        pr_info!("[rs_hello]: init end!\n");

        Ok(FrameWork { })
    }
}

impl Drop for FrameWork {
    fn drop(&mut self) {
    }
}
