#![no_std]
#![feature(asm_const)]
#![feature(naked_functions)]
#![feature(const_trait_impl)]
#![feature(const_maybe_uninit_zeroed)]

mod platform;

//pub mod arch;
//pub mod irq;
pub mod mem;
pub mod time;
//pub mod trap;

//pub mod paging;

pub mod console {
    pub use super::platform::console::*;
}

/*
pub mod misc {
    pub use super::platform::misc::*;
}
*/

struct LogIfImpl;

#[crate_interface::impl_interface]
impl axlog::LogIf for LogIfImpl {
    fn console_write_str(s: &str) {
        use crate::console::putchar;
        for c in s.chars() {
            match c {
                '\n' => {
                    putchar(b'\r');
                    putchar(b'\n');
                }
                _ => putchar(c as u8),
            }
        }
    }

    fn current_time() -> core::time::Duration {
        crate::time::current_time()
    }

    fn current_task_id() -> Option<u64> {
        #[cfg(feature = "multitask")]
        {
            axtask::current_may_uninit().map(|curr| curr.id().as_u64())
        }
        #[cfg(not(feature = "multitask"))]
        {
            None
        }
    }
}

struct GuardIfImpl;

#[crate_interface::impl_interface]
impl spinlock::GuardIf for GuardIfImpl {
    fn set_preemptible(_enabled: bool) {
        #[cfg(feature = "multitask")]
        axtask::set_preemptiable(_enabled);
    }
}
