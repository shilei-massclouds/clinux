#![no_std]
#![no_main]

#[macro_use]
extern crate libax;

use core::str::FromStr;

use libax::io::{self, prelude::*};
use libax::net::{IpAddr, TcpStream};

const DEST_IP: &str = "49.12.234.183"; // ident.me
const REQUEST: &str = "\
GET / HTTP/1.1\r\n\
Host: ident.me\r\n\
Accept: */*\r\n\
\r\n";

fn client() -> io::Result {
    let (addr, port) = (IpAddr::from_str(DEST_IP).unwrap(), 80);
    let mut stream = TcpStream::connect((addr, port).into())?;
    stream.write(REQUEST.as_bytes())?;

    let mut buf = [0; 1024];
    let n = stream.read(&mut buf)?;
    let response = core::str::from_utf8(&buf[..n]).unwrap();
    println!("{}", response);

    Ok(())
}

#[no_mangle]
fn test_httpclient() {
    println!("Hello, simple http client!");
    client().expect("test http client failed");
}
