#[derive(Debug, Clone)]
pub struct WorldMap {
    pub lin_speed: [f32; 3],
    pub rot_speed: [f32; 3],
    pub location: [f32; 2],
    pub current_alt: f32,
    pub roll: f32,
    pub pitch: f32,
    pub yaw: f32,
}

impl Default for WorldMap {
    fn default() -> Self {
        WorldMap {
            lin_speed: [0.0; 3],
            rot_speed: [0.0; 3],
            location: [0.0; 2],
            current_alt: 0.0,
            roll: 0.0,
            pitch: 0.0,
            yaw: 0.0,
        }
    }
}

pub fn find_pattern<T: PartialEq>(data: Vec<T>, pattern: Vec<T>) -> i64 {
    let mut index= -1;
    let mut same_elements= 0;
    if data.len() >= pattern.len() {
        for i in 0..=data.len() - pattern.len() {
            //IF he have detected the first element of the pattern
            if(data[i] == pattern[0] && index == -1) {
                //Save the index of the first element
                index = i as i64;
                for j in 0..pattern.len() {
                    if (data[i+j] == pattern[j]) {
                        same_elements += 1;
                    } else {
                        same_elements = 0;
                        index = -1;
                        break;
                    }
                }
            } 
            if (same_elements == pattern.len()) {
                break;
            }
        }
    }
    return index;
}