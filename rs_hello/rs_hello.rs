// SPDX-License-Identifier: GPL-2.0

use kernel::prelude::*;
use kernel::print::printk;
use crate::component::Component;

mod component;

trait IBase {
    fn ready(&self) -> bool;
    fn name(&self) -> *const core::ffi::c_char;
}

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
        unsafe {
            printk(b"module[RustHello]: init begin...\n\0");
            printk(b"module[RustHello]: init end!\n\0");
        }

        Ok(FrameWork { com: Component {} })
    }
}

impl Drop for FrameWork {
    fn drop(&mut self) {
    }
}

