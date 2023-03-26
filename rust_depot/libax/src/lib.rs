#![cfg_attr(not(test), no_std)]

pub use axlog::{debug, error, info, print, println, trace, warn};

extern crate alloc;

#[macro_use]
extern crate axlog;

#[cfg(not(test))]
extern crate axruntime;

pub mod io;
pub mod rand;
pub mod sync;
pub mod time;

pub mod task;

#[cfg(feature = "net")]
pub mod net;

#[cfg(feature = "display")]
pub mod display;
