#![allow(unused_variables)]

pub mod ethernet_interface;

use ethernet_interface::EthernetInterface;

fn main() {
    println!("Hello, world!");
    let eth = EthernetInterface::new("192.168.1.173".into(), 8080, true);
    //eth.listen();
    eth.send("Hello from Rust!".into(), "192.168.1.221:8080");
}
