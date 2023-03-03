pub trait ILib {
    fn find_next_bit(&self, bitmap: *const core::ffi::c_ulong, bitmap_size: core::ffi::c_uint, offset: core::ffi::c_uint) -> core::ffi::c_uint;
}
