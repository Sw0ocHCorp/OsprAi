use std::fs::{create_dir};
pub struct LogsRecorder {
    logs_file: Option<std::fs::File>,
    logs_file_path: String,
    data_receiver: Option<tokio::sync::broadcast::Receiver<String>>,
}

impl LogsRecorder {
    pub fn new(logs_file_path: String) -> LogsRecorder {
        let logs_dir_path = logs_file_path.clone();
        let logs_dir_split = logs_dir_path.split('.').collect::<Vec<&str>>();
        let logs_dir_base = logs_dir_split[0].to_string();
        let mut logs_dir_parts = logs_dir_base.split('/').collect::<Vec<&str>>();
        logs_dir_parts= logs_dir_parts[1..logs_dir_parts.len()-1].to_vec();
        let mut logs_dir_final = String::new();
        for part in logs_dir_parts {
            logs_dir_final.push('/');
            logs_dir_final.push_str(part);
        }
        match create_dir(logs_dir_final.clone()) {
            Ok(_) => {
                println!("Logs directory: {}\ncreated successfully", logs_dir_final);
            }
            Err(e) => {
                if e.kind() == std::io::ErrorKind::AlreadyExists {
                    println!("Logs directory: {}\nalready exists", logs_dir_final);
                } else {
                    eprintln!("Failed to create logs directory: {}", e);
                }
            }
        } 
        return LogsRecorder { logs_file: None, logs_file_path: logs_dir_final, data_receiver: None };
        //create_dir(path)
    }

    pub fn start_recording(&mut self, data_receiver: tokio::sync::broadcast::Receiver<String>) {
        let logs_file = std::fs::File::create(self.logs_file_path.clone());
        match logs_file {
            Ok(file) => {
                self.logs_file = Some(file);
                self.data_receiver = Some(data_receiver);

                println!("Logs file: {}\ncreated successfully", self.logs_file_path);
            }
            Err(e) => {
                if e.kind() == std::io::ErrorKind::AlreadyExists {
                    println!("Logs file: {}\nalready exists", self.logs_file_path);
                } else {
                    eprintln!("Failed to create logs file: {}", e);
                }
            }
        }
    }
}