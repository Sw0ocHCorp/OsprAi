use slint::{ComponentHandle, SharedString};
use std::{thread::{self, JoinHandle}};
use crate::{ethernet_interface::EthernetInterface};
use std::sync::{Arc, Mutex};
use chrono::{DateTime, Local};
use plotters::prelude::*;
use slint::SharedPixelBuffer;
//use tokio::sync::broadcast::{Receiver};
use crate::event_management::MessageType;
use crate::logs_recorder::LogsRecorder;
use crate::gamepad_manager::GamePadManager;
slint::slint!(import { OsprAiWindow } from "ui/osprai_window.slint";);

pub struct OsprAiSoftware {
    // All the UI elements are wrapped in an Arc<Mutex> to allow safe concurrent access in all callbacks
    ui: OsprAiWindow,
    eth: Arc<Mutex<EthernetInterface>>,
    logs_recorder: Arc<LogsRecorder>, // Optional logs recorder
    gamepad_manager: Option<Arc<Mutex<GamePadManager>>>, // Optional keyboard manager
    //current_perception: WorldMap,
    update_routine: Option<JoinHandle<()>>,
    is_drone_init:bool,
}

impl OsprAiSoftware {
    pub fn new() -> OsprAiSoftware {
        let ui = OsprAiWindow::new().expect("Failed to create UI");
        //let map: WorldMap = ui.get_perception_data();
        let eth = Arc::new(Mutex::new(EthernetInterface::new("192.168.1.235".into(), 8080, "192.168.1.109".into(), 8080)));
        let now = Local::now();
        let now_format= now.format("%Y-%m-%d_%H:%M:%S").to_string();
        let logs_recorder = Arc::new(LogsRecorder::new(format!("/home/sw0och/OsprAi/Controller/logs/{}.txt", now_format))); // Initialize logs recorder with a file path
        OsprAiSoftware {
            ui,
            eth,
            logs_recorder,
            gamepad_manager: Some(Arc::new(Mutex::new(GamePadManager::new()))), // Initialize keyboard manager
            //current_perception: map,
            update_routine: None,
            is_drone_init: false,
        }
    }
    
    pub fn start_listening_updates(&mut self) {
        let mut eth_receiver = self.eth.lock().unwrap().get_module_observer(); // Get a new observer for the Ethernet interface events
        let ui_handle: slint::Weak<OsprAiWindow> = self.ui.as_weak(); // Get a weak reference to the UI (to allow ui modification without ownership issues)
        self.update_routine = Some(thread::spawn( move ||{
            loop {
                if let Ok(msg) = eth_receiver.try_recv() {
                    match msg {
                        MessageType::UDPFrame(data) => {
                            //Using the weak_ref to put ui modification task in the queue of the event loop (let slint handle it)
                            match ui_handle.upgrade_in_event_loop(move|ui| {
                                                                    println!("Received data: {}", data);
                                                                    let mut perception= ui.get_perception_data();
                                                                    perception.lat += 1.0; // Simulate some change in perception data
                                                                    ui.set_perception_data(perception.clone());
                               
                            }) {
                                Ok(_) => {},
                                Err(e) => {
                                    println!("Failed to update perception data: {}", e);
                                }
                            }
                            
                        }
                        _ => {
                            println!("Received command: {:#?}", msg);
                        }
                    }
                }
            }
        }));
    }

    pub fn run(&mut self) -> Result<(), slint::PlatformError> {
        let eth_to_arm = self.eth.clone();
        let gmpad_task = self.gamepad_manager.clone();
        if let Some(gmpad_arc) = gmpad_task {
            gmpad_arc.lock().unwrap().start_listening();
        } else {
            println!("Keyboard manager is not initialized");
        }
        eth_to_arm.lock().unwrap().start(); // Start the Ethernet interface
        let ui_handle = self.ui.as_weak(); // Get a weak reference to the UI (to allow ui modification without ownership issues)
        let cam_display_size = self.ui.get_cam_display_size();

        self.ui.on_render_plot(move || {
            let mut pixel_buffer = SharedPixelBuffer::new(cam_display_size.width as u32, cam_display_size.height as u32);
            let backend = BitMapBackend::with_buffer(pixel_buffer.make_mut_bytes(), (cam_display_size.width as u32, cam_display_size.height as u32));
            let root = backend.into_drawing_area();
            root.fill(&WHITE).expect("error filling drawing area");
            let mut chart = ChartBuilder::on(&root).margin(10)
                .caption("OsprAi Orientation(in °)", ("arial", 15))
                .set_label_area_size(LabelAreaPosition::Left, 40)
                // enable X axis, the size is 40 px
                .set_label_area_size(LabelAreaPosition::Bottom, 40)
                .build_cartesian_2d(-180.0..180.0, -180.0..180.0)
                .unwrap();
            chart.configure_mesh().draw().expect("error drawing mesh");
            //chart.configure_axes().draw().expect("error drawing");
            chart.draw_series(LineSeries::new(
                (0..=1000).map(|x| {
                    let x = (x as f64 +1.0/1000.0)*180.0;
                    (x, x.sin())
                }),
                &RED,
            ))
            .expect("error drawing series");
            root.present().expect("error presenting");
            drop(chart);
            drop(root);
            match ui_handle.upgrade_in_event_loop(|ui| {
                                                    ui.set_monitoring_display(slint::Image::from_rgb8(pixel_buffer));
            }) {
                Ok(_) => {
                    println!("Plot image updated successfully");
                },
                Err(e) => {
                    println!("Failed to update plot image: {}", e);
                }
            }
        });

        let ui_handle = self.ui.as_weak(); // Get a weak reference to the UI (to allow ui modification without ownership issues)
        
        self.ui.on_arm_drone(move || {
            let eth = eth_to_arm.lock().unwrap(); // Lock the access(from other threads) during the callback
            eth.send(String::from("Arm"));
            //Using the weak_ref to put ui modification task in the queue of the event loop (let slint handle it)
            match ui_handle.upgrade_in_event_loop(|ui| {
                                                    ui.set_arm_msg(SharedString::from("DisArm\nDrone"));
                                                        //println!("{}, {}", ui.get_cam_display_size().width, ui.get_cam_display_size().height);
            }) {
                Ok(_) => {},
                Err(e) => println!("Failed to disarm drone: {}", e),
            }        
        });
            
        let ui_handle = self.ui.as_weak(); // Get a weak reference to the UI (to allow ui modification without ownership issues)
        let eth_to_disarm = self.eth.clone();
        let mut is_drone_init = self.is_drone_init.clone(); // Capture the current state of drone initialization
        let mut ui_copy= self.ui.clone_strong(); // Clone the UI to use in the callback
        self.ui.on_disarm_drone(move || {
            let eth = eth_to_disarm.lock().unwrap(); // Lock the access(from other threads) during the callback
            eth.send(String::from("DisArm"));
            //Using the weak_ref to put ui modification task in the queue of the event loop (let slint handle it)
            match ui_handle.upgrade_in_event_loop(|gui| {
                                                        gui.set_arm_msg(SharedString::from("Arm\nDrone"));
                                                        
            }) {
                Ok(_) => {
                    if (is_drone_init == false) {
                        is_drone_init = true;
                        ui_copy.invoke_render_plot();
                    }
                },
                Err(e) => println!("Failed to disarm drone: {}", e),
            }
        });

        self.ui.on_start_logs_record(|| {
            println!("Start recording logs");
        });

        self.ui.on_stop_logs_record(|| {
            println!("Stop recording logs");
        });
        let gmpad_clone = self.gamepad_manager.clone();
        let gmap_obs= gmpad_clone.unwrap().lock().unwrap().get_module_observer();
        let eth_clone = self.eth.clone();
        eth_clone.lock().unwrap().attach_external_observer(gmap_obs); // Attach the gamepad manager observer to the Ethernet interface
        //self.start_listening_updates(); // Start listening for updates from the Ethernet interface
        return self.ui.run();
    }
}