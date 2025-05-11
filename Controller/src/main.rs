#![allow(unused_variables)]
#![allow(unused_imports)]
pub mod ethernet_interface;
mod events_management;

use bevy::prelude::*;
use bevy_egui::{egui, EguiContexts, EguiPlugin, EguiContextPass};
use events_management::{CustomEvent, Event, Observer};

fn main() {
    let mut test_event = events_management::CustomEvent::new();
    test_event.add_observer(events_management::Observer::new());
    test_event.trigger("Hello".to_string());
}
