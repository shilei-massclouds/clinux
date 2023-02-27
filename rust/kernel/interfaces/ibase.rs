pub trait IBase {
    fn ready(&self) -> bool;
    fn name(&self) -> *const core::ffi::c_char;
}
