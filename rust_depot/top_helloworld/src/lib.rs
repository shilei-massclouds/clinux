#![no_std]
#![no_main]

#[no_mangle]
fn say_hello() {
    libax::println!("Hello, world!");
}
