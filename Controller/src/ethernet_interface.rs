
use std::clone;
use std::sync::{Arc, Mutex};
use std::thread::{self, JoinHandle};
use std::net::UdpSocket;

use plotters::data;
use tokio::sync::broadcast::{Receiver};

use crate::event_management::{MessageType, CustomEvent};

#[allow(dead_code)]
pub struct EthernetInterface {
    src_address: String,
    src_port: u16,
    target_address: String, // Optional target address for sending data
    target_port: u16, // Optional target port for sending data
    socket: Arc<UdpSocket>, // Smart pointer to the UdpSocket (allows shared ownership, to use in threads)
    event: Arc<CustomEvent<MessageType>>, // Message manager for handling messages
    observer: Arc<Mutex<Option<Receiver<MessageType>>>>, // Observer for receiving messages from other modules
    ethernet_routine: Option<JoinHandle<()>>,

}

impl EthernetInterface {
    pub fn new(src_address: String, src_port: u16, target_address: String, target_port:u16) -> EthernetInterface {
        let sock = UdpSocket::bind(format!("{}:{}", src_address, src_port)).expect("Try to bind Socket to src_address");
        sock.set_nonblocking(true).expect("Failed to set socket to non-blocking mode");
        return EthernetInterface {src_address: src_address, src_port: src_port, target_address: target_address, target_port: target_port, socket: Arc::new(sock), 
                                    event: Arc::new(CustomEvent::new()), ethernet_routine: None, observer: Arc::new(Mutex::new(None))};
    }
    
    pub fn start(&mut self) {
        let socket_clone = self.socket.clone();
        let event_clone = self.event.clone();
        let observer_clone = self.observer.clone();
        let target_address = self.target_address.clone();
        let target_port = self.target_port.clone();
        self.ethernet_routine = Some(thread::spawn(move || {
            loop {
                Self::listen(socket_clone.clone(), event_clone.clone());
                let mut obs = observer_clone.lock().unwrap();
                match *obs { 
                    Some(ref mut observ) => {
                        if let Ok(msg) = observ.try_recv() {
                            match msg {
                                MessageType::UDPFrame(frame) => {
                                    socket_clone.send_to(frame.as_bytes(), format!("{}:{}", target_address, target_port)).expect("Trying to send UDPFrame");
                                }
                                MessageType::ControllerCmd(cmd) => {
                                    socket_clone.send_to(Self::msg_to_string(msg).as_bytes(), format!("{}:{}", target_address, target_port)).expect("Trying to send COntrollerCmd");
                                }
                            }
                        }
                    }
                    None => {
                    }
                    
                }
            }
            /*loop {
                let observer_clone = observer_clone.clone(); // Clone the observer to use in the thread
                let obs= observer_clone.lock().unwrap().take();
                let test = obs.is_none();
                Self::listen(socket_clone.clone(), event_clone.clone());
                if let Some(obs) = observer_clone.clone().lock().unwrap().take() {
                    println!("Observer is attached");   
                }
            }*/
        }));
    }

    pub fn listen(socket: Arc<UdpSocket>, frame_event: Arc<CustomEvent<MessageType>>) {    
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
    
    pub fn msg_to_string(msg: MessageType) -> String {
        match msg {
            MessageType::UDPFrame(data) => { return data }
            MessageType::ControllerCmd(cmd) => {
                return format!("ControllerCmd: x_left: {}, y_left: {}, x_right: {}, y_right: {}, button_south: {}, button_east: {}, button_west: {}, button_north: {}",
                               cmd.x_left, cmd.y_left, cmd.x_right, cmd.y_right,
                               cmd.button_south, cmd.button_east, cmd.button_west, cmd.button_north);
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
        let mut obs = self.observer.lock().unwrap();
        *obs = Some(observer);
    }
}