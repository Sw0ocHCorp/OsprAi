use std::collections::HashMap;

use plotters::data;

use crate::utils;

static FLOAT_SIZE: usize = 4;

#[derive(Clone, Copy, PartialEq)]
enum ParsingStep {
    FrameSize,
    DataId,
    DataSize,
    Data,
    CheckSum,
}

#[derive(Clone)]
pub struct FrameParser {
    sof: Vec<u8>,
    parsing_ids: HashMap<Vec<u8>, String>
}

impl FrameParser {
    pub fn new(sof: Vec<u8>, parsing_ids:HashMap<Vec<u8>, String>) -> FrameParser {
        FrameParser {
            sof,
            parsing_ids: parsing_ids,
        }
    }

    pub fn parse_frame(&self, mut frame: Vec<u8>) -> HashMap<String, Vec<f32>> {
        let mut parsed_data: HashMap<String, Vec<f32>> = HashMap::new();
        let mut current_state= ParsingStep::FrameSize;
        let mut buffer = Vec::<u8>::new();
        let mut data_id = String::new();
        let mut frame_size= 0;
        let mut data_size = 0;
        let mut checksum: u8 = 0;
        let mut remain_bytes= -1;
        let starting_index= utils::find_pattern(frame.clone(), self.sof.clone());
        if starting_index > 0 {
            for _ in 0..starting_index {
                let __= frame.remove(0);
            }
        }
        for byte in &self.sof {
            checksum = checksum.wrapping_add(*byte);
        }
        for byte in &frame[self.sof.clone().len()..] {
            buffer.push(*byte);
            match current_state {
                ParsingStep::FrameSize => {
                    frame_size= *byte as i16;
                    remain_bytes= frame_size - self.sof.len() as i16;
                    buffer.clear();
                    current_state = ParsingStep::DataId;
                    checksum = checksum.wrapping_add(*byte);
                },
                ParsingStep::DataId => {
                    checksum = checksum.wrapping_add(*byte);
                    for id in self.parsing_ids.keys() {
                        if buffer == *id {
                            parsed_data.insert(self.parsing_ids[id].clone(), vec![]);
                            data_id = self.parsing_ids[id].clone();
                            buffer.clear();
                            current_state = ParsingStep::DataSize;
                            break;
                        }
                    }
                }
                ParsingStep::DataSize => {
                    checksum = checksum.wrapping_add(*byte);
                    data_size= *byte as i16;
                    buffer.clear();
                    current_state = ParsingStep::Data;
                },
                ParsingStep::Data => {
                    checksum = checksum.wrapping_add(*byte);
                    if (buffer.len() >= data_size as usize) {
                        for i in (0..buffer.len()).step_by(FLOAT_SIZE) {
                            let float_val = f32::from_le_bytes([buffer[i], buffer[i+1], buffer[i+2], buffer[i+3]]);
                            if let Some(data_vec) = parsed_data.get_mut(&data_id) {
                                data_vec.push(float_val);
                            }
                        }
                        buffer.clear();
                        data_size= 0;
                        data_id= String::new();
                        if (remain_bytes <= 2) {
                            current_state = ParsingStep::CheckSum;
                        } else {
                            current_state = ParsingStep::DataId;
                        }
                    }

                }, 
                ParsingStep::CheckSum => {
                    if checksum != *byte {
                        println!("Checksum invalid");
                        parsed_data.clear();
                    }
                    break;
                }
            }
            remain_bytes -= 1;
        }
        if (current_state != ParsingStep::CheckSum) {
            println!("Frame incomplete");
            parsed_data.clear();
        }
        return parsed_data;
    }

    pub fn encode_frame(&self, data: HashMap<Vec<u8>, Vec<f32>>) -> Vec<u8> {
        let mut encoded_frame = self.sof.clone();
        encoded_frame.push(0x00); // Placeholder for frame size
        for (id, values) in data {
            encoded_frame.extend(id.iter());
            encoded_frame.push((values.len() * FLOAT_SIZE) as u8);
            for val in &values {
                encoded_frame.extend(val.to_be_bytes());
            }
        }
        let mut checksum: u8 = 0;
        for byte in &encoded_frame {
            checksum = checksum.wrapping_add(*byte);
        }
        checksum= checksum.wrapping_add(encoded_frame.len() as u8 + 1);
        encoded_frame.push(checksum);
        encoded_frame[self.sof.len()]= encoded_frame.len() as u8;
        
        return encoded_frame;
    }
}