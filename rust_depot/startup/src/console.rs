pub fn putchar(c: u8) {
    #[allow(deprecated)]
    sbi_rt::legacy::console_putchar(c as usize);
}

pub fn puts(s: &str) {
    for c in s.chars() {
        match c {
            '\n' => {
                putchar(b'\r');
                putchar(b'\n');
            }
            _ => putchar(c as u8),
        }
    }
}

pub fn getchar() -> Option<u8> {
    #[allow(deprecated)]
    match sbi_rt::legacy::console_getchar() as isize {
        -1 => None,
        c => Some(c as u8),
    }
}
