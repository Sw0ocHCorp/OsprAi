use gilrs::{ev, Button, Gilrs};
use std::{sync::Arc, thread::{self, JoinHandle}};
use tokio::sync::broadcast::{Receiver};
use crate::event_management::{MessageType, CustomEvent};

#[derive(Debug, Clone, Copy)]
pub struct GamepadCmd {
    pub x_left: f32,
    pub y_left: f32,
    pub x_right: f32,
    pub y_right: f32,
    pub button_south: bool,
    pub button_east: bool,
    pub button_west: bool,
    pub button_north: bool,
}

pub struct GamePadManager {
    listening_thread: Option<JoinHandle<()>>,
    input_event: Arc<CustomEvent<MessageType>>,
}

impl GamePadManager {
    pub fn new() -> GamePadManager {
        return GamePadManager {listening_thread: None, input_event: Arc::new(CustomEvent::new())};
    }
    pub fn start_listening(&mut self) {
        let mut gl = Gilrs::new().unwrap();
        let mut gmpad_id = None;
        let input_event = self.input_event.clone();
        if (gl.gamepads().count() > 0) {
            println!("External Gamepad detected");
            self.listening_thread = Some(thread::spawn(move || {
                loop {
                    if let Some(e) = gl.next_event() {
                        if gmpad_id.is_none() {
                            gmpad_id = Some(e.id);
                        } else if let Some(id) = gmpad_id {
                            let gamepad = gl.gamepad(id);
                            let mut y_left = 0.0;
                            if gamepad.axis_data(ev::Axis::LeftStickY).is_none() == false{
                                y_left = gamepad.axis_data(ev::Axis::LeftStickY).unwrap().value();
                                if y_left.abs() < 0.1 {
                                    y_left = 0.0; // Deadzone for left stick Y
                                }
                            }
                            let mut x_left = 0.0;
                            if gamepad.axis_data(ev::Axis::LeftStickX).is_none() == false {
                                x_left = gamepad.axis_data(ev::Axis::LeftStickX).unwrap().value();
                                if x_left.abs() < 0.1 {
                                    x_left = 0.0; // Deadzone for left stick X
                                }
                            }
                            let mut y_right = 0.0;
                            if gamepad.axis_data(ev::Axis::RightStickY).is_none() == false {
                                y_right = gamepad.axis_data(ev::Axis::RightStickY).unwrap().value();
                                if y_right.abs() < 0.1 {
                                    y_right = 0.0; // Deadzone for right stick Y
                                }
                            }
                            let mut x_right = 0.0;
                            if gamepad.axis_data(ev::Axis::RightStickX).is_none() == false {
                                x_right = gamepad.axis_data(ev::Axis::RightStickX).unwrap().value();
                                if x_right.abs() < 0.1 {
                                    x_right = 0.0; // Deadzone for right stick X
                                }
                            }
                            let cmd= GamepadCmd { x_left, y_left, x_right, y_right,
                                button_south: gamepad.is_pressed(Button::South), button_east: gamepad.is_pressed(Button::East),
                                button_west: gamepad.is_pressed(Button::West), button_north: gamepad.is_pressed(Button::North),
                            };
                            input_event.trigger(MessageType::ControllerCmd(cmd));
                        }
                    }
                }
            }));
        }
        else {
            println!("No external Gamepad detected, using Main Sticks and Buttons");
        }
    }

    pub fn stop_listening(&mut self) {
        if let Some(handle) = self.listening_thread.take() {
            handle.join().expect("Failed to join keyboard listening thread");
        } else {
            println!("No keyboard listening thread to stop.");
        }
    }

    pub fn get_module_observer(&self) -> Receiver<MessageType> {
        return self.input_event.get_new_observer();
    }
}