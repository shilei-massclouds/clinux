pub trait ILib {
    fn find_next_bit(&self,
                     addr: *const u64,
                     size: u64,
                     offset: u64) -> u64;
}
