// SPDX-License-Identifier: GPL-2.0

//! Allocator support.

//use core::ptr;

/*
use core::alloc::{GlobalAlloc, Layout};
struct KernelAllocator;

unsafe impl GlobalAlloc for KernelAllocator {
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        // `krealloc()` is used instead of `kmalloc()` because the latter is
        // an inline function and cannot be bound to as a result.
        unsafe { bindings::krealloc(ptr::null(), layout.size(), bindings::GFP_KERNEL) as *mut u8 }
    }

    unsafe fn dealloc(&self, ptr: *mut u8, _layout: Layout) {
        unsafe {
            bindings::kfree(ptr as *const core::ffi::c_void);
        }
    }
}

#[global_allocator]
static ALLOCATOR: KernelAllocator = KernelAllocator;
*/

extern "C" {
    pub static GFP_KERNEL_C: gfp_t;
}

extern "C" {
    pub static __GFP_ZERO_C: gfp_t;
}

// `rustc` only generates these for some crate types. Even then, we would need
// to extract the object file that has them from the archive. For the moment,
// let's generate them ourselves instead.
//
// Note that `#[no_mangle]` implies exported too, nowadays.
#[no_mangle]
fn __rust_alloc(size: usize, _align: usize) -> *mut u8 {
    unsafe { krealloc(core::ptr::null(), size, GFP_KERNEL_C) as *mut u8 }
}

#[no_mangle]
fn __rust_dealloc(ptr: *mut u8, _size: usize, _align: usize) {
    unsafe { kfree(ptr as *const core::ffi::c_void) };
}

#[no_mangle]
fn __rust_realloc(ptr: *mut u8, _old_size: usize, _align: usize, new_size: usize) -> *mut u8 {
    unsafe {
        krealloc(
            ptr as *const core::ffi::c_void,
            new_size,
            GFP_KERNEL_C,
        ) as *mut u8
    }
}

#[no_mangle]
fn __rust_alloc_zeroed(size: usize, _align: usize) -> *mut u8 {
    unsafe {
        krealloc(
            core::ptr::null(),
            size,
            GFP_KERNEL_C | __GFP_ZERO_C,
        ) as *mut u8
    }
}

#[allow(non_camel_case_types)]
pub type gfp_t = core::ffi::c_uint;

extern "C" {
    pub fn krealloc(
        objp: *const core::ffi::c_void,
        new_size: usize,
        flags: gfp_t,
    ) -> *mut core::ffi::c_void;
}

extern "C" {
    pub fn kfree(objp: *const core::ffi::c_void);
}

#[no_mangle]
fn __rust_alloc_error_handler(_size: usize, _align: usize) -> ! {
    loop {}
}

#[no_mangle]
#[allow(non_upper_case_globals)]
static __rust_alloc_error_handler_should_panic: u8 = 1;
