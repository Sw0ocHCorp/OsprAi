
use std::clone;
use std::collections::HashMap;
use std::sync::{Arc, Mutex};
use std::thread::{self, JoinHandle};
use std::net::UdpSocket;

use plotters::data;
use tokio::sync::broadcast::{Receiver};

use crate::event_management::{MessageType, CustomEvent};
use crate::frame_parser::FrameParser;
use crate::utils::WorldMap;

#[allow(dead_code)]
pub struct EthernetInterface {
    src_address: String,
    src_port: u16,
    target_address: String, // Optional target address for sending data
    target_port: u16, // Optional target port for sending data
    event: Arc<CustomEvent<MessageType>>, // Message manager for handling messages
    
    frame_received_obs: Vec<Arc<Mutex<Receiver<MessageType>>>>, // List of Observer for receiving messages from other modules
    ethernet_routine: Option<JoinHandle<()>>,
    parser: FrameParser,
}

impl EthernetInterface {
    pub fn new(src_address: String, src_port: u16, target_address: String, target_port:u16, parser: FrameParser) -> EthernetInterface {
        return EthernetInterface {src_address: src_address, src_port: src_port, target_address: target_address, target_port: target_port, 
                                    event: Arc::new(CustomEvent::new()), ethernet_routine: None, frame_received_obs: Vec::new(), parser: parser};
    }
    
    pub fn start(&mut self) {
        let mut socket: Option<Arc<UdpSocket>>= None;
        let event_clone = self.event.clone();
        let frame_received_obs_clone = self.frame_received_obs.clone();
        let target_address = self.target_address.clone();
        let target_port = self.target_port.clone();
        let src_address = self.src_address.clone();
        let src_port = self.src_port.clone();
        let parser = self.parser.clone();
        let mut is_connected= false;
        self.ethernet_routine = Some(thread::spawn(move || {
            loop {
                if is_connected == false {
                    match Self::connect(src_address.clone(), src_port) {
                        Some(s) => {
                            socket = Some(Arc::new(s));
                            event_clone.trigger(MessageType::UDPAddressAndPort(src_address.clone(), src_port));
                            is_connected= true;
                        },
                        None => {
                            event_clone.trigger(MessageType::UDPAddressAndPort("".to_string(), 0));
                            thread::sleep(std::time::Duration::from_secs(1));
                            is_connected= false;
                        }
                        
                    }

                } else {
                    match socket.clone() {
                        Some(sock) => {
                            for obs in frame_received_obs_clone.clone() {
                                if let Ok(msg) = obs.lock().unwrap().try_recv() {
                                    match msg {
                                        MessageType::UDPFrame(frame) => { /*socket_clone.send_to(frame.as_bytes(), format!("{}:{}", target_address, target_port)).expect("Trying to send UDPFrame");*/ }
                                        _ =>{
                                            let frame = Self::msg_to_frame(msg, &parser);
                                            is_connected= Self::send(sock.clone(), target_address.clone(), target_port, frame);
                                            if is_connected == false {
                                                event_clone.trigger(MessageType::UDPAddressAndPort("".to_string(), 0));
                                            }
                                            //s.lock().unwrap().send_to(&frame, format!("{}:{}", target_address, target_port)).expect("Trying to send UDPFrame");
                                        }
                                    }
                                }
                            }
                            Self::listen(sock.clone(), event_clone.clone(), parser.clone());
                        }, 
                        None => {
                            is_connected= false;
                        }
                    }
                }
            }
        }));
    }

    fn connect( src_address: String, src_port: u16) -> Option<UdpSocket> { 
        match UdpSocket::bind(format!("{}:{}", src_address, src_port)) {
            Ok(s) => {
                s.set_nonblocking(true).expect("Failed to set socket to non-blocking mode");
                return Some(s);
            },
            Err(e) => { 
                return None;
            }
        };
    }

    fn listen(socket: Arc<UdpSocket>, worldmap_sender_event: Arc<CustomEvent<MessageType>>, parser: FrameParser) -> bool {    
        let mut buf = [0; 1024];
        match socket.recv_from(&mut buf) {
            Ok((size, src)) => {
                let data= parser.parse_frame(buf[..size].to_vec());
                if data.len() > 0 {
                    let wm= Self::frame_data_to_worldmap(data);
                    worldmap_sender_event.trigger(MessageType::WMap(wm));
                } 
                return true;
                //frame_event.trigger(MessageType::UDPFrame(String::from("LOOPBACK")));
            },
            _ => {
                return false;
            }
        }
    }
    pub fn send(socket: Arc<UdpSocket>, target_address: String, target_port: u16, data: Vec<u8>) -> bool {
        match socket.send_to(&data, format!("{}:{}", target_address, target_port)) {
            Ok(_) => {
                return true;
            },
            Err(_) => {
                return false;
            }
        } 
    }

    pub fn frame_data_to_worldmap(data: HashMap<String, Vec<f32>>) -> WorldMap {
        let mut wm= WorldMap::default();
        for (key, value) in data {
            if key.contains("lin_speed") && value.len() == 3 {
                wm.lin_speed= [value[0], value[1], value[2]];
            } else if key.contains("rot_speed") && value.len() == 3 {
                wm.rot_speed= [value[0], value[1], value[2]];
            } else if key.contains("current_alt") && value.len() == 1 {
                wm.current_alt= value[0];
            } else if key.contains("lat_lon") && value.len() == 2 {
                wm.location= [value[0], value[1]];
            }
        }

        return wm;
    }

    pub fn msg_to_frame(msg: MessageType, parser: &FrameParser) -> Vec<u8> {
        match msg {
            //MessageType::UDPFrame(data) => { return data }
            MessageType::ControllerCmd(cmd) => {
                let mut pad_inputs: HashMap<Vec<u8>, Vec<f32>> = HashMap::from([(vec![0x00, 0x0B], vec![cmd.x_left, cmd.y_left, cmd.x_right, cmd.y_right])]);
                if cmd.button_south {
                    pad_inputs.insert(vec![0x00, 0x0A], vec![10.0]);
                }
                else if cmd.button_east {
                    pad_inputs.insert(vec![0x00, 0x0A], vec![15.0]);
                }
                return parser.encode_frame(pad_inputs);
            }
            MessageType::GUICmd(cmd) => {
                let mut ui_inputs: HashMap<Vec<u8>, Vec<f32>> = HashMap::new();
                if cmd.need_arm {
                    ui_inputs.insert(vec![0x00, 0x0A], vec![10.0]);
                }
                else {
                    ui_inputs.insert(vec![0x00, 0x0A], vec![15.0]);
                }
                return parser.encode_frame(ui_inputs);
            }, 
            _ => {
                return Vec::new();
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
        self.frame_received_obs.push(Arc::new(Mutex::new(observer)));
    }
    
}