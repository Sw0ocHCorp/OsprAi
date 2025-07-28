use std::collections::HashMap;

use plotters::data;

#[derive(Clone, Copy)]
enum ParsingStep {
    Sof,
    FrameSize,
    DataId,
    DataSize,
    Data,
    CheckSum,
}

pub struct FrameParser {
    sof: String,
    parsing_ids: HashMap<String, String>,
    encoding_ids: HashMap<String, String>,
}

impl FrameParser {
    pub fn new(sof: String, parsing_ids:HashMap<String, String>, encoding_ids : HashMap<String, String>) -> FrameParser {
        FrameParser {
            sof,
            parsing_ids: parsing_ids,
            encoding_ids: encoding_ids,
        }
    }

    pub fn parse_frame(&self, frame: String) -> HashMap<String, Vec<f32>> {
        let mut parsed_data: HashMap<String, Vec<f32>> = HashMap::new();
        let mut current_state= ParsingStep::Sof;
        let mut buffer = String::new();
        let mut data_id = String::new();
        let mut frame_size= 0;
        let mut data_size = 0;
        let mut checksum: u8 = 0;

        for i in (0..frame.len()).step_by(2) {
            frame_size -= 1;
            buffer.push_str(&frame[i..i+2]);
            if frame_size <= 0 && current_state.clone() as i8 > ParsingStep::FrameSize as i8 {
                current_state = ParsingStep::CheckSum;
            } else {
                let lastByte = buffer[buffer.len()-2..].to_string();
                checksum= checksum.wrapping_add(u8::from_str_radix(&lastByte, 16).unwrap());
            }
            match current_state {
                ParsingStep::Sof => {
                    if buffer == self.sof {
                        buffer.clear();
                        current_state = ParsingStep::FrameSize;
                    }
                },
                ParsingStep::FrameSize => {
                    frame_size += i16::from_str_radix(&buffer, 16).unwrap();
                    buffer.clear();
                    current_state = ParsingStep::DataId;
                }, 
                ParsingStep::DataId => {
                    if self.parsing_ids.contains_key(&buffer) {
                        parsed_data.insert(self.parsing_ids.get(&buffer).unwrap().clone(), Vec::new());
                        data_id = self.parsing_ids.get(&buffer).unwrap().clone();
                        buffer.clear();
                        current_state = ParsingStep::DataSize;
                    }
                    
                },
                ParsingStep::DataSize => {
                    data_size = i16::from_str_radix(&buffer, 16).unwrap();
                    buffer.clear();
                    current_state = ParsingStep::Data;
                }, 
                ParsingStep::Data => {
                    //Float Data
                    if (buffer.len() / 2) % 4 == 0 && data_size % 4 == 0 {
                        let float_val = f32::from_bits(u32::from_str_radix(&buffer, 16).unwrap());
                        buffer.clear();
                        let mut n_data= 0;
                        if let Some(data_vec) = parsed_data.get_mut(&data_id) {
                            data_vec.push(float_val);
                            n_data = data_vec.len();
                        }
                        if n_data == data_size as usize / 4 {
                            current_state = ParsingStep::DataId;
                        }
                        
                    }
                    //Int Data
                    else if (buffer.len() / 2) % 2 == 0 && data_size % 2 == 0 && data_size % 4 != 0 {
                        let int_val = i16::from_str_radix(&buffer, 16).unwrap();
                        buffer.clear();
                        let mut n_data= 0;
                        if let Some(data_vec) = parsed_data.get_mut(&data_id) {
                            data_vec.push(int_val as f32);
                            n_data = data_vec.len();
                        }
                        if n_data == data_size as usize / 2 {
                            current_state = ParsingStep::DataId;
                        }
                    }
                    
                },
                ParsingStep::CheckSum => {
                    if checksum == u8::from_str_radix(&buffer, 16).unwrap() {
                        println!("Checksum valid for frame: {}", frame);
                        return parsed_data;
                    } else {
                        println!("Checksum invalid for frame: {}", frame);
                    }
                }
            }
        }
        parsed_data.clear();
        return parsed_data;
    }

    pub fn encode_frame(&self, data: HashMap<String, Vec<f32>>) -> String {
        let mut encoded_frame = self.sof.clone() + "00";
        let mut checksum: u8 = 0;
        for i in (0..self.sof.len()).step_by(2) {
            let last_byte = self.sof[i..i+2].to_string();
            checksum = checksum.wrapping_add(u8::from_str_radix(&last_byte, 16).unwrap());
        }
        for (id, values) in data {
            if let Some(encoded_id) = self.encoding_ids.get(&id) {
                for i in (0..encoded_id.len()).step_by(2) {
                    let last_byte = encoded_id[i..i+2].to_string();
                    checksum = checksum.wrapping_add(u8::from_str_radix(&last_byte, 16).unwrap());
                }
                encoded_frame += &encoded_id;
                let data_size = values.len() * 4; // Assuming float values
                encoded_frame += &format!("{:02x}", data_size);
                checksum = checksum.wrapping_add(data_size as u8);
                for value in values {
                    let bits_value = value.to_bits();
                    let hex_value = format!("{:08x}", bits_value);
                    for i in (0..hex_value.len()).step_by(2) {
                        let last_byte = hex_value[i..i+2].to_string();
                        checksum = checksum.wrapping_add(u8::from_str_radix(&last_byte, 16).unwrap());
                    }
                    encoded_frame += &hex_value;
                }
            }
        }
        let frame_size = (encoded_frame.len() / 2) + 1;
        checksum = checksum.wrapping_add(frame_size as u8);
        encoded_frame += &format!("{:02x}", checksum);
        encoded_frame= encoded_frame.replace(&(self.sof.clone() + "00"),&(self.sof.clone() + &format!("{:02x}", frame_size)));
        return encoded_frame;
    }
}