// SPDX-License-Identifier: GPL-2.0

//! Component [rs_hello].

use kernel::prelude::*;
use kernel::interfaces::ibase::IBase;
use crate::component::Component;

mod ilib_stub;
mod component;

provide! {
    interface: IBase,
    component: FrameWork,
}

module! {
    type: FrameWork,
    name: "rust_hello",
    author: "Rust for Linux Contributors",
    description: "Rust hello_world sample",
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
        pr_info!("module[RustHello]: init begin...\n");
        pr_info!("module[RustHello]: init end!\n");

        Ok(FrameWork { com: Component {} })
    }
}

impl Drop for FrameWork {
    fn drop(&mut self) {
    }
}
