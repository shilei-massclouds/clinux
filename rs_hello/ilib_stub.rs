//! Stub for [ILib]

use kernel::interfaces::ilib::ILib;

struct Stub;

impl ILib for Stub {
    fn find_next_bit(&self, bitmap: *const core::ffi::c_ulong, bitmap_size: core::ffi::c_uint, offset: core::ffi::c_uint, ) -> core::ffi::c_uint {
        unsafe { ilib_find_next_bit(bitmap, bitmap_size, offset, ) }
    }

}

pub(crate) fn get_ilib() -> impl ILib {
    Stub {}
}

extern "C" {

    pub(crate) fn ilib_find_next_bit(bitmap: *const core::ffi::c_ulong, bitmap_size: core::ffi::c_uint, offset: core::ffi::c_uint, ) -> core::ffi::c_uint;
    
}
