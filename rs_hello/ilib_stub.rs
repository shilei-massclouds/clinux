use core::ffi::c_ulong;
use kernel::interfaces::ilib::ILib;

struct Stub;

impl ILib for Stub {
    fn find_next_bit(&self, addr: *const c_ulong, size: c_ulong,
                     offset: c_ulong)
        -> c_ulong {
        unsafe {
            lib_find_next_bit(addr, size, offset)
        }
    }
}

pub(crate) fn get_ilib() -> impl ILib {
    Stub {}
}

extern "C" {
    pub(crate) fn lib_find_next_bit(addr: *const c_ulong, size: c_ulong,
                                    offset: c_ulong)
        -> c_ulong;
}
