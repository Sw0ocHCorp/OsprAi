
use std::sync::{Arc, Mutex};
use std::thread::{self, JoinHandle};
use std::net::UdpSocket;

use tokio::sync::broadcast::{channel, Sender, Receiver};

use crate::event_management::{MessageType, Event};


#[allow(dead_code)]
pub struct EthernetInterface {
    address: String,
    port: u16,
    socket: Arc<UdpSocket>, // Smart pointer to the UdpSocket (allows shared ownership, to use in threads)
    event: Arc<Event<MessageType>>, // Message manager for handling messages
    observer: Option<Arc<Mutex<Receiver<MessageType>>>>, // Observer for receiving messages from other modules
    ethernet_routine: Option<JoinHandle<()>>,

}

impl EthernetInterface {
    pub fn new(address: String, port: u16) -> EthernetInterface {
        let sock = UdpSocket::bind(format!("{}:{}", address, port)).expect("Try to bind Socket to address");
        sock.set_nonblocking(true).expect("Failed to set socket to non-blocking mode");
        return EthernetInterface {address: address, port: port, socket: Arc::new(sock), 
                                    event: Arc::new(Event::new()), ethernet_routine: None, observer: None};
    }
    
    pub fn start(&mut self) {
        let socket_clone = self.socket.clone();
        let event_clone= self.event.clone();
        let observer_clone = self.observer.clone();
        self.ethernet_routine = Some(thread::spawn(move || {
            loop {
                Self::listen(socket_clone.clone(), event_clone.clone());
                if let Some(observer_arc) = &observer_clone {
                    if let Ok(msg) = observer_arc.lock().unwrap().try_recv() {
                        match msg {
                            MessageType::Text(data) => {
                                println!("Received data: {}", data);
                            },
                            MessageType::Binary(cmd) => {
                                println!("Received command: {:#?}", cmd);
                            }
                        }
                    }
                }
            }
        }));
    }

    pub fn listen(socket: Arc<UdpSocket>, frame_event: Arc<Event<MessageType>>) {    
        let mut buf = [0; 1024];
        match socket.recv_from(&mut buf) {
            Ok(((size, src))) => {
                println!("Received {} bytes from {}: {:?}", size, src, &buf[..size]);
                frame_event.trigger(MessageType::Text(String::from("LOOPBACK")));
            },
            _ => {
            }
        }
    }
    pub fn send(&self, data: String, dest: &str) {
        self.socket.send_to(data.as_bytes(), dest).expect("Trying to send data");
    }
    
    pub fn waitEnd(&mut self) {
        if let Some(handle) = self.ethernet_routine.take() {
            handle.join().expect("Failed to join ethernet thread");
        }
    }

    pub fn getModuleObserver(&self) -> Receiver<MessageType> {
        return self.event.getNewObserver();
    }
    
    pub fn attachExternalObserver(&mut self, observer: Arc<Mutex<Receiver<MessageType>>>) {
        self.observer = Some(observer);
    }
}