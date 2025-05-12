use bevy::ecs::observer;

#[allow(dead_code)]
pub struct Observer<T> {
    _marker: std::marker::PhantomData<T>,
    callback_fun: Option<Box<dyn Fn(&T)>>,
}
#[allow(dead_code)]
impl<T> Observer<T> {
    pub fn new() -> Self {
        let obs= Observer { _marker: std::marker::PhantomData, callback_fun: None };
        return obs;
    }
    
    pub fn set_callback<F: Fn(&T) + 'static>(&mut self, f: F) {
        // Set the callback function
        self.callback_fun = Some(Box::new(f));
    }

    fn respond(&self, data: &T) {
        if self.callback_fun.is_some() {
            // Call the callback function with the data
            (self.callback_fun.as_ref().unwrap())(data);
        } else {
            // Handle the case where no callback function is set
            std::println!("No callback function set for this observer.");
        }
    }
}
#[allow(dead_code)]
pub trait SpecificCallbacks {
    fn test(&self, data: &String) {
        println!("Received data: {}", data);
    }
}


#[allow(dead_code)]
//------------- GENERIC EVENTS & OBSERVERS -------------//
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

//------------- SPECIFIC EVENTS & OBSERVERS -------------//