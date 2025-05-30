use crate::ethernet_interface::EthernetInterface;
use std::sync::{Arc, Mutex};

slint::slint!(import { OsprAiWindow } from "ui/osprai_window.slint";);

pub struct OsprAiSoftware {
    // All the UI elements are wrapped in an Arc<Mutex> to allow safe concurrent access in all callbacks
    ui: Arc<Mutex<OsprAiWindow>>,
    eth: Arc<Mutex<EthernetInterface>>,
    pub update_available: Arc<Mutex<bool>>,
    pub update_progress: f32, // 0.0 to 1.0
}

impl OsprAiSoftware {
    pub fn new() -> OsprAiSoftware {
        let ui = Arc::new(Mutex::new(OsprAiWindow::new().expect("Failed to create UI")));
        let eth = Arc::new(Mutex::new(EthernetInterface::new("127.0.0.1".into(), 8080)));
        OsprAiSoftware {
            ui,
            eth,
            update_available: Arc::new(Mutex::new(false)),
            update_progress: 0.0,
        }
    }
        
    pub fn run(&mut self) -> Result<(), slint::PlatformError> {
        {
            let mut eth_to_arm = self.eth.clone();
            
            let mut ui= self.ui.lock().unwrap();
            let update_available = self.update_available.clone(); // Get a cloned instance of the smart pointer
            eth_to_arm.lock().unwrap().start(); // Start the Ethernet interface
            ui.on_arm_drone(move || {
                let mut eth = eth_to_arm.lock().unwrap(); // Lock the access(from other threads) to the 
                eth.send(String::from("Arm"), String::from("127.0.0.1"), 8000);
                let mut update_available = update_available.lock().unwrap(); // Lock the access(from other threads) to the data
                println!("{} Arm drone clicked", update_available);
                *update_available = !*update_available; // Simulate an update being available
            });
            let mut eth_to_disarm = self.eth.clone();
             ui.on_disarm_drone(move || {
                let mut eth = eth_to_disarm.lock().unwrap(); // Lock the access(from other threads) to the 
                eth.send(String::from("DisArm"), String::from("127.0.0.1"), 8000);
            });
            ui.on_start_logs_record(|| {
                println!("Start recording logs");
            });
            ui.on_stop_logs_record(|| {
                println!("Stop recording logs");
            });
        }
        return self.ui.lock().unwrap().run();
    }
}