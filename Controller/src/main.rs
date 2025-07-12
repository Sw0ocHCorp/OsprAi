//slint::include_modules!();
pub mod ethernet_interface;
pub mod event_management;
pub mod ui;
pub mod logs_recorder;
pub mod gamepad_manager;
use ui::OsprAiSoftware;



fn main()  -> Result<(), slint::PlatformError> {
    let mut soft = OsprAiSoftware::new();
    return soft.run();
}