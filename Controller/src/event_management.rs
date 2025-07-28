use tokio::sync::broadcast::{channel, Sender, Receiver};
use crate::gamepad_manager::GamepadCmd;
use crate::ui::UICmd;

#[derive(Debug, Clone)]
pub enum MessageType {
    UDPFrame(String),
    ControllerCmd(GamepadCmd),
    GUICmd(UICmd)
}

pub struct CustomEvent<T: Clone> {
    sender: Sender<T>,
}

impl<T: Clone> CustomEvent<T> {
    pub fn new() -> CustomEvent<T> {
        let (sender, _) = channel::<T>(100);
        CustomEvent {sender}
    }

    pub fn get_new_observer(&self) ->Receiver<T> {
        // Logic to add observer
        return self.sender.subscribe();
    }

    pub fn trigger(&self, data: T) {
        // Logic to trigger event
        let _ = self.sender.send(data);
    }
} 