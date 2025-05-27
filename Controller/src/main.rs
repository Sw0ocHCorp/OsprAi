/*#![allow(unused_variables)]
#![allow(unused_imports)]
pub mod ethernet_interface;
mod events_management;

use std::sync::mpsc;
use bevy::prelude::*;
use bevy_egui::{egui, EguiContexts, EguiPlugin, EguiContextPass};
use events_management::{CustomEvent, Observer};
use ethernet_interface::EthernetInterface;
use std::sync::{Arc, Mutex};

fn main() {
    
    let eth = Arc::new(Mutex::new(EthernetInterface::new("192.168.1.173".into(), 8080, true)));
    let mut test_event = events_management::CustomEvent::new();
    let mut observer = events_management::Observer::new();
    let eth_clone = Arc::clone(&eth);
    observer.set_callback(move |msg: &String| {
        if let Ok(mut eth) = eth_clone.lock() {
            eth.test(msg);
        }
    });
    test_event.add_observer(observer);
    if let Ok(mut eth_guard) = eth.lock() {
        eth_guard.set_event(test_event);
        // eth_guard.listen();
    }
}*/

pub mod ethernet_interface;
pub mod message_type;

use ethernet_interface::EthernetInterface;
use std::sync::mpsc::{self, Sender, Receiver};

fn main() {
    println!("Hello, world!");
    let mut eth = EthernetInterface::new("127.0.0.1".into(), 8080);
    eth.start();
    println!("Listening ");
    //eth.send("Hello from Rust!".into(), "127.0.0.1:8080");
}
