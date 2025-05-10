
use std::net::UdpSocket;
#[allow(dead_code)]
pub struct EthernetInterface {
    address: String,
    port: u16,
    is_client: bool,
    socket: UdpSocket,
}

impl EthernetInterface {
    pub fn new(address: String, port: u16, is_client: bool) -> EthernetInterface {
        let sock = UdpSocket::bind(format!("{}:{}", address, port)).expect("Try to bind Socket to address");
        return EthernetInterface { address: address, port: port, is_client: is_client, socket: sock }
    }
    
    pub fn listen(&self) {    
        let mut buf = [0; 1024];
        loop {
            let (size, src) = self.socket.recv_from(&mut buf).expect("Failed to receive data");
            println!("Received {} bytes from {}: {:?}", size, src, &buf[..size]);
        }
    }
    pub fn send(&self, data: String, dest: &str) {
        self.socket.send_to(data.as_bytes(), dest).expect("Trying to send data");
    }
}