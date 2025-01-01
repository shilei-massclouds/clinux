#![no_std]
#![no_main]

#[macro_use]
extern crate libax;

use core::sync::atomic::{AtomicUsize, Ordering};
use libax::task;

const NUM_TASKS: usize = 10;
static FINISHED_TASKS: AtomicUsize = AtomicUsize::new(0);

#[no_mangle]
fn test_yield() {
    for i in 0..NUM_TASKS {
        task::spawn(move || {
            println!("Hello, task {}! id = {:?}", i, task::current().id());

            task::yield_now();

            let order = FINISHED_TASKS.fetch_add(1, Ordering::Relaxed);
            assert!(order == i); // FIFO scheduler
        });
    }
    println!("Hello, main task!");
    while FINISHED_TASKS.load(Ordering::Relaxed) < NUM_TASKS {
        task::yield_now();
    }
    println!("Task yielding tests run OK!");
}
