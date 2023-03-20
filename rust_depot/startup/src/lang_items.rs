use core::panic::PanicInfo;

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    //error!("{}", info);
    crate::misc::terminate()
}
