#![cfg_attr(not(feature = "std"), no_std)]

extern crate log;

#[cfg(not(feature = "std"))]
use crate_interface::{call_interface, def_interface};

use core::fmt::{self, Write};
use core::str::FromStr;

use log::{Level, LevelFilter, Log, Metadata, Record};

pub use log::{debug, error, info, trace, warn};

#[macro_export]
macro_rules! print {
    ($fmt: literal $(, $($arg: tt)+)?) => {
        $crate::__print_impl(format_args!($fmt $(, $($arg)+)?));
    }
}

#[macro_export]
macro_rules! println {
    () => { print!("\n") };
    ($fmt: literal $(, $($arg: tt)+)?) => {
        $crate::__print_impl(format_args!(concat!($fmt, "\n") $(, $($arg)+)?));
    }
}

macro_rules! with_color {
    ($color_code:expr, $($arg:tt)*) => {{
        format_args!("\u{1B}[{}m{}\u{1B}[m", $color_code as u8, format_args!($($arg)*))
    }};
}

#[repr(u8)]
#[allow(dead_code)]
enum ColorCode {
    Black = 30,
    Red = 31,
    Green = 32,
    Yellow = 33,
    Blue = 34,
    Magenta = 35,
    Cyan = 36,
    White = 37,
    BrightBlack = 90,
    BrightRed = 91,
    BrightGreen = 92,
    BrightYellow = 93,
    BrightBlue = 94,
    BrightMagenta = 95,
    BrightCyan = 96,
    BrightWhite = 97,
}

/// Extern interfaces called in this crate.
#[cfg(not(feature = "std"))]
#[def_interface]
pub trait LogIf {
    /// write a string to the console.
    fn console_write_str(s: &str);

    /// get current time
    fn current_time() -> core::time::Duration;

    /// get current task ID.
    fn current_task_id() -> Option<u64>;
}

struct Logger;

impl Write for Logger {
    fn write_str(&mut self, s: &str) -> fmt::Result {
        cfg_if::cfg_if! {
            if #[cfg(feature = "std")] {
                std::print!("{}", s);
            } else {
                call_interface!(LogIf::console_write_str, s);
            }
        }
        Ok(())
    }
}

impl Log for Logger {
    fn enabled(&self, _metadata: &Metadata) -> bool {
        true
    }

    fn log(&self, record: &Record) {
        if !self.enabled(record.metadata()) {
            return;
        }

        let level = record.level();
        let line = record.line().unwrap_or(0);
        let target = record.target();
        let level_color = match level {
            Level::Error => ColorCode::BrightRed,
            Level::Warn => ColorCode::BrightYellow,
            Level::Info => ColorCode::BrightGreen,
            Level::Debug => ColorCode::BrightCyan,
            Level::Trace => ColorCode::BrightBlack,
        };
        let args_color = match level {
            Level::Error => ColorCode::Red,
            Level::Warn => ColorCode::Yellow,
            Level::Info => ColorCode::Green,
            Level::Debug => ColorCode::Cyan,
            Level::Trace => ColorCode::BrightBlack,
        };

        cfg_if::cfg_if! {
            if #[cfg(feature = "std")] {
                __print_impl(with_color!(
                    ColorCode::White,
                    "[{time} {level} {path} {args}\n",
                    time = chrono::Local::now().format("%Y-%m-%d %H:%M:%S%.6f"),
                    level = with_color!(level_color, "{:<5}", level),
                    path = with_color!(ColorCode::White, "{}:{}]", target, line),
                    args = with_color!(args_color, "{}", record.args()),
                ));
            } else {
                let tid = call_interface!(LogIf::current_task_id);
                let now = call_interface!(LogIf::current_time);
                if let Some(tid) = tid {
                    __print_impl(with_color!(
                        ColorCode::White,
                        "[{:>3}.{:06} {level} {tid} {path} {args}\n",
                        now.as_secs(),
                        now.subsec_micros(),
                        level = with_color!(level_color, "{:<5}", level),
                        tid = with_color!(ColorCode::BrightWhite, "{}", tid),
                        path = with_color!(ColorCode::White, "{}:{}]", target, line),
                        args = with_color!(args_color, "{}", record.args()),
                    ));
                } else {
                    __print_impl(with_color!(
                        ColorCode::White,
                        "[{:>3}.{:06} {level} {path} {args}\n",
                        now.as_secs(),
                        now.subsec_micros(),
                        level = with_color!(level_color, "{:<5}", level),
                        path = with_color!(ColorCode::White, "{}:{}]", target, line),
                        args =with_color!(args_color, "{}", record.args()),
                    ));
                };
            }
        }
    }

    fn flush(&self) {}
}

pub fn __print_impl(args: fmt::Arguments) {
    Logger.write_fmt(args).unwrap();
}

pub fn init() {
    log::set_logger(&Logger).unwrap();
    log::set_max_level(LevelFilter::Warn);
}

pub fn set_max_level(level: &str) {
    let lf = LevelFilter::from_str(level)
        .ok()
        .unwrap_or(LevelFilter::Off);
    log::set_max_level(lf);
}
