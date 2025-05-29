use tokio::sync::broadcast::{channel, Sender, Receiver};

#[derive(Debug, Clone)]
pub enum MessageType {
    UDPFrame(String),
    Binary(Vec<u8>),
}

pub struct Event<T: Clone> {
    sender: Sender<T>,
}

impl<T: Clone> Event<T> {
    pub fn new() -> Event<T> {
        let (sender, _) = channel::<T>(100);
        Event {sender}
    }

    pub fn getNewObserver(&self) ->Receiver<T> {
        // Logic to add observer
        return self.sender.subscribe();
    }

    pub fn trigger(&self, data: T) {
        // Logic to trigger event
        let _ = self.sender.send(data);
    }
} 