use std::sync::Arc;
use std::cell::{RefCell, OnceCell};
use bitflags::bitflags;
use std::sync::atomic::AtomicUsize;

bitflags! {
    /// Module Status (bitwise layout).
    pub struct ModuleStatus: usize {
        const INITIAL   = 0b00;
        const TOUCHED   = 0b01;
        const DONE      = 0b10;
    }
}

pub struct Module {
    pub name: String,
    pub status: AtomicUsize,
    pub max_deplen: AtomicUsize,
    pub nr_elements: OnceCell<usize>,
    pub undef_syms: RefCell<Vec<String>>,
    pub dependencies: RefCell<Vec<ModuleRef>>,
}

impl Module {
    pub fn new(name: &str) -> Self {
        Self {
            name: name.to_owned(),
            status: AtomicUsize::new(ModuleStatus::INITIAL.bits()),
            max_deplen: AtomicUsize::new(0),
            nr_elements: OnceCell::new(),
            undef_syms: RefCell::new(vec![]),
            dependencies: RefCell::new(vec![]),
        }
    }
}

pub type ModuleRef = Arc<Module>;
