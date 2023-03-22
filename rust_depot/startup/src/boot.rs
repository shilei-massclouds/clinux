use riscv::register::satp;

use axconfig::TASK_STACK_SIZE;

#[allow(dead_code)]
#[link_section = ".bss.stack"]
static mut BOOT_STACK: [u8; TASK_STACK_SIZE] = [0; TASK_STACK_SIZE];

#[link_section = ".data.boot_page_table"]
static mut BOOT_PT_SV39: [u64; 512] = [0; 512];

unsafe fn init_mmu() {
    // 0x0000_0000_8000_0000..0x0000_0000_c000_0000, VRWX_GAD, 1G block
    BOOT_PT_SV39[2] = (0x80000 << 10) | 0xef;
    // 0xffff_ffc0_8000_0000..0xffff_ffc0_c000_0000, VRWX_GAD, 1G block
    BOOT_PT_SV39[0x102] = (0x80000 << 10) | 0xef;

    /* Qemu pflash acts as the repository of modules,
     * startup loads modules from it.
     * The pflash is located at 0x22000000 in PA,
     * just setup identity-mapping for the first pgdir temporily. */
    // 0x0000_0000..0x4000_0000, VRW__GAD, 1G block
    BOOT_PT_SV39[0] = (0 << 10) | 0xe7;

    let page_table_root = BOOT_PT_SV39.as_ptr() as usize;
    satp::set(satp::Mode::Sv39, 0, page_table_root >> 12);
    riscv::asm::sfence_vma_all();
}

#[naked]
#[no_mangle]
#[link_section = ".text.boot"]
unsafe extern "C" fn _start() -> ! {
    extern "C" {
        fn load_modules();
    }

    // PC = 0x8020_0000
    // a0 = hartid
    core::arch::asm!("
        mv      s0, a0                  // 0. save hartid
        la      sp, {boot_stack_top}    // 1. set SP

        call    {init_mmu}              // 2. setup boot page table and enabel MMU

        // NOW: const PHYS_VIRT_OFFSET doesn't work here. Why?
        li      t1, 0xffffffc000000000  // fixit: Use config
        la      t0, 1f
        add     t0, t0, t1
        jr      t0
 1:                                     // jump point from pa to va

        la      sp, {boot_stack_top}    // *. reset SP

        la      a1, {platform_init}     // 3. fix up virtual high address
        jalr    a1                      // 4. call platform_init()

        la      a1, {load_modules}
        mv      a0, s0                  // 5. call load_modules()
        jalr    a1

        tail    {unreachable}",
        boot_stack_top = sym crate::mem::boot_stack_top,
        init_mmu = sym init_mmu,
        platform_init = sym super::platform_init,
        load_modules = sym load_modules,
        unreachable = sym unreachable,
        options(noreturn),
    )
}

fn unreachable()
{
    use crate::console::puts;

    puts("\n##########################");
    puts("\nImpossible to come here!\n");
    puts("##########################\n");

    sbi_rt::system_reset(sbi_rt::Shutdown, sbi_rt::NoReason);
}
