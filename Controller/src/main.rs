#![allow(unused_variables)]
#![allow(unused_imports)]
pub mod ethernet_interface;
mod events_management;

use bevy::prelude::*;
use bevy_egui::{egui, EguiContexts, EguiPlugin, EguiContextPass};
use events_management::{CustomEvent, Event, Observer};

fn main() {
    let mut test_event = events_management::CustomEvent::new();
    let mut observer = events_management::Observer::new();
    //We can pass Lambda Function or function of another impl as argument
    observer.set_callback(|data: &String| {
        println!("Received: {}", data);
    });
    test_event.add_observer(observer);
    test_event.trigger(&("Hello".to_string()));
}
