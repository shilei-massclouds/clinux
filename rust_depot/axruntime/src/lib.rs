#![cfg_attr(not(test), no_std)]

#[macro_use]
extern crate axlog;

const LOGO: &str = r#"
       d8888                            .d88888b.   .d8888b.
      d88888                           d88P" "Y88b d88P  Y88b
     d88P888                           888     888 Y88b.
    d88P 888 888d888  .d8888b  .d88b.  888     888  "Y888b.
   d88P  888 888P"   d88P"    d8P  Y8b 888     888     "Y88b.
  d88P   888 888     888      88888888 888     888       "888
 d8888888888 888     Y88b.    Y8b.     Y88b. .d88P Y88b  d88P
d88P     888 888      "Y8888P  "Y8888   "Y88888P"   "Y8888P"
"#;


#[cfg_attr(not(test), no_mangle)]
fn rust_main() {
    println!("{}", LOGO);
    println!(
        "\
        arch = {}\n\
        platform = {}\n\
        build_mode = {}\n\
        log_level = {}\n\
        ",
        option_env!("ARCH").unwrap_or(""),
        option_env!("PLATFORM").unwrap_or(""),
        option_env!("MODE").unwrap_or(""),
        option_env!("LOG").unwrap_or(""),
    );

    axlog::init();
    axlog::set_max_level(option_env!("LOG").unwrap_or("")); // no effect if set `log-level-*` features
    info!("Logging is enabled.");

    info!("Found physcial memory regions:");
    for r in axhal::mem::memory_regions() {
        println!(
            "  [{:x?}, {:x?}) {} ({:?})",
            r.paddr,
            r.paddr + r.size,
            r.name,
            r.flags
        );
    }

    info!("Initialize global memory allocator...");
    init_allocator();

    info!("Initialize kernel page table...");
    remap_kernel_memory().expect("remap kernel memoy failed");

    axtask::init_scheduler();

    let all_devices = axdriver::init_drivers();
    axnet::init_network(all_devices.net);

    #[cfg(any(feature = "fs", feature = "display"))]
    {
        #[cfg(feature = "display")]
        axdisplay::init_display(all_devices.display);
    }

    init_interrupt();
}

fn init_allocator() {
    use axhal::mem::{memory_regions, phys_to_virt, MemRegionFlags};

    let mut max_region_size = 0;
    let mut max_region_paddr = 0.into();
    for r in memory_regions() {
        if r.flags.contains(MemRegionFlags::FREE) && r.size > max_region_size {
            max_region_size = r.size;
            max_region_paddr = r.paddr;
        }
    }
    for r in memory_regions() {
        if r.flags.contains(MemRegionFlags::FREE) && r.paddr == max_region_paddr {
            axalloc::global_init(phys_to_virt(r.paddr).as_usize(), r.size);
            break;
        }
    }
    for r in memory_regions() {
        if r.flags.contains(MemRegionFlags::FREE) && r.paddr != max_region_paddr {
            axalloc::global_add_memory(phys_to_virt(r.paddr).as_usize(), r.size)
                .expect("add heap memory region failed");
        }
    }
}

fn remap_kernel_memory() -> Result<(), axhal::paging::PagingError> {
    use axhal::mem::{memory_regions, phys_to_virt};
    use axhal::paging::PageTable;

    let mut kernel_page_table = PageTable::try_new()?;
    for r in memory_regions() {
        kernel_page_table.map_region(
            phys_to_virt(r.paddr),
            r.paddr,
            r.size,
            r.flags.into(),
            true,
        )?;
    }

    unsafe { axhal::arch::write_page_table_root(kernel_page_table.root_paddr()) };
    core::mem::forget(kernel_page_table);
    Ok(())
}

fn init_interrupt() {
    use axhal::time::TIMER_IRQ_NUM;
    use core::sync::atomic::{AtomicU64, Ordering};

    // Setup timer interrupt handler
    const PERIODIC_INTERVAL_NANOS: u64 =
        axhal::time::NANOS_PER_SEC / axconfig::TICKS_PER_SEC as u64;
    static NEXT_DEADLINE: AtomicU64 = AtomicU64::new(0);

    fn update_timer() {
        let now_ns = axhal::time::current_time_nanos();
        let mut next_deadline = NEXT_DEADLINE.fetch_add(PERIODIC_INTERVAL_NANOS, Ordering::Acquire);
        if now_ns >= next_deadline {
            next_deadline = now_ns + PERIODIC_INTERVAL_NANOS;
            NEXT_DEADLINE.store(next_deadline + PERIODIC_INTERVAL_NANOS, Ordering::SeqCst);
        }
        axhal::time::set_oneshot_timer(next_deadline);
    }

    axhal::irq::register_handler(TIMER_IRQ_NUM, || {
        update_timer();
        axtask::on_timer_tick();
    });

    // Enable IRQs before starting app
    axhal::arch::enable_irqs();
}
