//slint::include_modules!();
pub mod ethernet_interface;
pub mod event_management;
pub mod ui;
pub mod logs_recorder;
pub mod gamepad_manager;
pub mod utils;
use std::time::{Duration, Instant};
use std::collections::HashMap;

use ui::OsprAiSoftware;

use crate::{frame_parser::FrameParser, utils::WorldMap};
pub mod frame_parser;


fn main()  -> Result<(), slint::PlatformError> {
    let mut soft = OsprAiSoftware::new();
    return soft.run();
}