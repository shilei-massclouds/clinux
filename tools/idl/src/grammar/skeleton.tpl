// SPDX-License-Identifier: GPL-2.0

//! Component [{{component}}]

use kernel::prelude::*;
use kernel::interfaces::{{module}}::{{interface}};
use crate::component::Component;

mod component;

provide! {
    interface: {{interface}},
    component: FrameWork,
}

module! {
    type: FrameWork,
    name: "{{component}}",
    author: "Rust for Linux Contributors",
    description: "Component {{component}}",
    license: "GPL",
}

struct FrameWork {
    com: Component,
}

impl {{interface}} for FrameWork {
    fn ready(&self) -> bool {
        self.com.ready()
    }

    fn name(&self) -> *const core::ffi::c_char {
        self.com.name()
    }
}

impl kernel::Module for FrameWork {
    fn init(_name: &'static CStr, _module: &'static ThisModule) -> Result<Self> {
        pr_info!("[{{component}}]: init begin...\n");
        pr_info!("[{{component}}]: init end!\n");

        Ok(FrameWork { com: Component {} })
    }
}

impl Drop for FrameWork {
    fn drop(&mut self) {
    }
}
