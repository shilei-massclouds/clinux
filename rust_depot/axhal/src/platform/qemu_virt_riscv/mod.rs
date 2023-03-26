pub mod console;
pub mod irq;
pub mod mem;
pub mod misc;
pub mod time;

#[no_mangle]
pub(crate) fn platform_init() {
    extern "C" {
        fn trap_vector_base();
    }
    crate::arch::set_tap_vector_base(trap_vector_base as usize);
    self::irq::init();
    self::time::init();
}
