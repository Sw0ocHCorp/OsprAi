use bevy::ecs::observer;

#[allow(dead_code)]
pub struct Observer {}

impl Observer {
    pub fn new() -> Self {
        let custom_observer = Observer {};
        return custom_observer;
    }
    fn respond(&self, data: String) {
        std::println!("Observer received data: {}", data);
    }
}

#[allow(dead_code)]
pub trait Event {
    fn new() -> Self;
    fn trigger(&self, data: String) {
    }
    fn add_observer(&mut self, observer: Observer);
}
#[allow(dead_code)]
pub struct CustomEvent {
    observers: Vec<Observer>,
}

impl Event for CustomEvent {
    fn new() -> Self {
        let custom_event = CustomEvent {
            observers: Vec::new(),
        };
        return custom_event;
    }
    fn add_observer(&mut self, observer: Observer) {
        self.observers.push(observer);
    }
    
    fn trigger(&self, data: String) {
        for obs in self.observers.iter() {
            obs.respond(data.clone());
        }
    }
}