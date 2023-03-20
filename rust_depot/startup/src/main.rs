#![no_std]
#![no_main]
#![feature(asm_const)]
#![feature(naked_functions)]
#![feature(const_trait_impl)]
#![feature(const_maybe_uninit_zeroed)]

mod boot;
mod arch;
mod misc;
mod lang_items;

pub mod console;
/*
pub mod irq;
pub mod misc;
pub mod time;
*/
pub mod mem;

pub(crate) fn platform_init() {
    extern "C" {
        fn trap_vector_base();
    }
    crate::mem::clear_bss();
    crate::arch::set_tap_vector_base(trap_vector_base as usize);
    //self::irq::init();
    //self::time::init();
}
