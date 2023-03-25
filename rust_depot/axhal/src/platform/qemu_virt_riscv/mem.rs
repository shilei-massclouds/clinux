use crate::mem::*;

pub(crate) fn memory_regions_num() -> usize {
    common_memory_regions_num() + 1
}

pub(crate) fn memory_region_at(idx: usize) -> Option<MemRegion> {
    use core::cmp::Ordering;
    match idx.cmp(&common_memory_regions_num()) {
        Ordering::Less => common_memory_region_at(idx),
        Ordering::Equal => {
            // free memory
            extern "C" {
                fn skernel();
                static kernel_size: usize;
            }
            let start = skernel as usize + unsafe {kernel_size};
            let start = virt_to_phys(start.into()).align_up_4k();
            let end = PhysAddr::from(axconfig::PHYS_MEMORY_END).align_down_4k();
            Some(MemRegion {
                paddr: start,
                size: end.as_usize() - start.as_usize(),
                flags: MemRegionFlags::FREE | MemRegionFlags::READ | MemRegionFlags::WRITE,
                name: "free memory",
            })
        }
        Ordering::Greater => None,
    }
}
