use bevy::ecs::observer;

#[allow(dead_code)]
pub struct Observer<T> {
    _marker: std::marker::PhantomData<T>,
}

impl<T> Observer<T> {
    pub fn new() -> Self {
        let obs= Observer { _marker: std::marker::PhantomData };
        return obs;
    }
    fn respond(&self, data: &T) {
        std::println!("Observer received data:");
    }
}

#[allow(dead_code)]
pub trait Event<T> {
    fn new() -> Self;
    fn trigger(&self, data: &T) {
    }
    fn add_observer(&mut self, observer: Observer<T>);
}
#[allow(dead_code)]
pub struct CustomEvent<T> {
    observers: Vec<Observer<T>>,
}

impl<T> Event<T> for CustomEvent<T> {
    fn new() -> Self {
        let custom_event = CustomEvent {
            observers: Vec::new(),
        };
        return custom_event;
    }
    fn add_observer(&mut self, observer: Observer<T>) {
        self.observers.push(observer);
    }
    
    fn trigger(&self, data: &T) {
        for obs in self.observers.iter() {
            obs.respond(data);
        }
    }
}