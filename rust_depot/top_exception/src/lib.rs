#![no_std]
#![no_main]

use core::arch::asm;
use libax::println;

fn rasie_break_exception() {
    unsafe {
        asm!("ebreak");
    }
}

#[no_mangle]
fn test_exception() {
    println!("Running exception tests...");
    rasie_break_exception();
    println!("Exception tests run OK!");
}
