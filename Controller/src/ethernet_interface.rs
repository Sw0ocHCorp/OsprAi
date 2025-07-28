
use std::clone;
use std::collections::HashMap;
use std::sync::{Arc, Mutex};
use std::thread::{self, JoinHandle};
use std::net::UdpSocket;

use plotters::data;
use tokio::sync::broadcast::{Receiver};

use crate::event_management::{MessageType, CustomEvent};
use crate::frame_parser::FrameParser;

#[allow(dead_code)]
pub struct EthernetInterface {
    src_address: String,
    src_port: u16,
    target_address: String, // Optional target address for sending data
    target_port: u16, // Optional target port for sending data
    socket: Arc<UdpSocket>, // Smart pointer to the UdpSocket (allows shared ownership, to use in threads)
    event: Arc<CustomEvent<MessageType>>, // Message manager for handling messages
    observers: Vec<Arc<Mutex<Receiver<MessageType>>>>, // List of Observer for receiving messages from other modules
    ethernet_routine: Option<JoinHandle<()>>,
    parser: FrameParser,
}

impl EthernetInterface {
    pub fn new(src_address: String, src_port: u16, target_address: String, target_port:u16, parser: FrameParser) -> EthernetInterface {
        let sock = UdpSocket::bind(format!("{}:{}", src_address, src_port)).expect("Try to bind Socket to src_address");
        sock.set_nonblocking(true).expect("Failed to set socket to non-blocking mode");
        return EthernetInterface {src_address: src_address, src_port: src_port, target_address: target_address, target_port: target_port, socket: Arc::new(sock), 
                                    event: Arc::new(CustomEvent::new()), ethernet_routine: None, observers: Vec::new(), parser: parser};
    }
    
    pub fn start(&mut self) {
        let socket_clone = self.socket.clone();
        let event_clone = self.event.clone();
        let observers_clone = self.observers.clone();
        let target_address = self.target_address.clone();
        let target_port = self.target_port.clone();
        let parser = self.parser.clone();
        self.ethernet_routine = Some(thread::spawn(move || {
            loop {
                Self::listen(socket_clone.clone(), event_clone.clone());
                let mut obss = observers_clone.clone();
                for obs in obss {
                    if let Ok(msg) = obs.lock().unwrap().try_recv() {
                        match msg {
                            MessageType::UDPFrame(frame) => { socket_clone.send_to(frame.as_bytes(), format!("{}:{}", target_address, target_port)).expect("Trying to send UDPFrame"); }
                            _ =>{
                                let frame = Self::msg_to_string(msg, &parser);
                                socket_clone.send_to(frame.as_bytes(), format!("{}:{}", target_address, target_port)).expect("Trying to send UDPFrame");
                            }
                        }
                    }
                }
            }
        }));
    }

    fn listen(socket: Arc<UdpSocket>, frame_event: Arc<CustomEvent<MessageType>>) {    
        let mut buf = [0; 1024];
        match socket.recv_from(&mut buf) {
            Ok((size, src)) => {
                println!("Received {} bytes from {}: {:?}", size, src, &buf[..size]);
                frame_event.trigger(MessageType::UDPFrame(String::from("LOOPBACK")));
            },
            _ => {
            }
        }
    }
    pub fn send(&self, data: String) {
        println!("Sending data: {}", data);
        self.socket.send_to(data.as_bytes(), format!("{}:{}", self.target_address, self.target_port)).expect("Trying to send data");
    }
    
    pub fn msg_to_string(msg: MessageType, parser: &FrameParser) -> String {
        match msg {
            MessageType::UDPFrame(data) => { return data }
            MessageType::ControllerCmd(cmd) => {
                let mut pad_inputs = HashMap::from([("000b".to_string(), vec![cmd.x_left, cmd.y_left, cmd.x_right, cmd.y_right])]);
                if cmd.button_south {
                    pad_inputs.insert("000a".to_string(), vec![10.0]);
                }
                else if cmd.button_east {
                    pad_inputs.insert("000a".to_string(), vec![15.0]);
                }
                return parser.encode_frame(pad_inputs);
                /*let mut pad_inputs: Vec<f32>= Vec::new();
                if cmd.button_south {
                    pad_inputs.push(10.0);
                }
                else {
                    pad_inputs.push(15.0)
                }
                pad_inputs.extend_from_slice(&[cmd.x_left, cmd.y_left, cmd.x_right, cmd.y_right]);
                return self.parser.encode_frame(HashMap::from([("000b", pad_inputs)]));*/
                /*return format!("ControllerCmd: x_left: {}, y_left: {}, x_right: {}, y_right: {}, button_south: {}, button_east: {}, button_west: {}, button_north: {}",
                               cmd.x_left, cmd.y_left, cmd.x_right, cmd.y_right,
                               cmd.button_south, cmd.button_east, cmd.button_west, cmd.button_north);*/
            }
            MessageType::GUICmd(cmd) => {
                let mut ui_inputs = HashMap::new();
                if cmd.need_arm {
                    ui_inputs.insert("000a".to_string(), vec![10.0]);
                }
                else {
                    ui_inputs.insert("000a".to_string(), vec![15.0]);
                }
                return parser.encode_frame(ui_inputs);
            }
        }
    }

    pub fn wait_end(&mut self) {
        if let Some(handle) = self.ethernet_routine.take() {
            handle.join().expect("Failed to join ethernet thread");
        }
    }

    pub fn get_module_observer(&self) -> Receiver<MessageType> {
        return self.event.get_new_observer();
    }
    
    pub fn attach_external_observer(&mut self, observer: Receiver<MessageType>) {
        self.observers.push(Arc::new(Mutex::new(observer)));
    }
    
}