use std::sync::{Arc, Mutex};
use std::thread::{self, JoinHandle};
use std::sync::mpsc::{self, Sender, Receiver};
use std::net::UdpSocket;

use crate::message_type::MessageType;


#[allow(dead_code)]
pub struct EthernetInterface {
    data: String,
    address: String,
    port: u16,
    socket: Arc<UdpSocket>, // Using Arc to allow shared ownership across threads (To use in the thread in start method)
    frame_sender: Sender<MessageType>,
    frame_receiver: Receiver<MessageType>,
    ethernet_routine: Option<JoinHandle<()>>,
}

impl EthernetInterface {
    pub fn new(address: String, port: u16) -> EthernetInterface {
        let sock = UdpSocket::bind(format!("{}:{}", address, port)).expect("Try to bind Socket to address");
        let (frame_sender, frame_receiver) = mpsc::channel::<MessageType>();
        return EthernetInterface {data: "".to_string(), address: address, port: port, socket: Arc::new(sock), 
                                    frame_sender: frame_sender, frame_receiver: frame_receiver, ethernet_routine: None};
    }
    
    pub fn start(&mut self) {
        let socket_clone= self.socket.clone();
        self.ethernet_routine = Some(thread::spawn(move || {
            Self::listen(socket_clone);
        }));
    }

    pub fn listen(socket: Arc<UdpSocket>) {    
        let mut buf = [0; 1024];
        loop {
            let (size, src) = socket.recv_from(&mut buf).expect("ERROR receiving data");
            println!("Received {} bytes from {}: {:?}", size, src, &buf[..size]);
        }
    }
    pub fn send(&self, data: String, dest: &str) {
        self.socket.send_to(data.as_bytes(), dest).expect("Trying to send data");
    }
    pub fn test(&mut self, msg: &String) {
        println!("TEST Observer: {}", msg);
        println!("data value -> {}", self.data);
        self.data= msg.to_string();
        println!("data value -> {}", self.data);
    }
    /*pub fn set_event(&mut self, ev: CustomEvent<String>) {
        self.listen_event = Some(ev);
    }*/
}