//slint::include_modules!();
pub mod ethernet_interface;
pub mod event_management;
pub mod ui;
pub mod logs_recorder;
pub mod gamepad_manager;
use std::collections::HashMap;

use ui::OsprAiSoftware;

use crate::frame_parser::FrameParser;
pub mod frame_parser;

fn main() {
    let mut parser = FrameParser::new("abcd".to_string(), HashMap::from([("0011".to_string(), "test0".to_string()), ("00a1".to_string(), "test1".to_string()), 
                                                                                            ("0044".to_string(), "test2".to_string()), ("00ce".to_string(), "id3".to_string())]),
                                                                            HashMap::from([("encod0".to_string(), "00f1".to_string()), ("encod1".to_string(), "00a2".to_string()), 
                                                                                            ("encod2".to_string(), "00b3".to_string()), ("encod3".to_string(), "00c4".to_string())]));
    let mut parser2 = FrameParser::new("abcd".to_string(), HashMap::from([("00f1".to_string(), "encod0".to_string()), ("00a2".to_string(), "encod1".to_string()), 
                                                                                            ("00b3".to_string(), "encod2".to_string()), ("00c4".to_string(), "encod3".to_string())]),
                                                                            HashMap::from([("0011".to_string(), "test0".to_string()), ("00a1".to_string(), "test1".to_string()), 
                                                                                            ("0044".to_string(), "test2".to_string()), ("00ce".to_string(), "id3".to_string())]));
    /*FrameParser parser("abcd", {{"0011", "test0"}, {"00a1", "test1"},{"0044", "test2"},{"00ce", "id3"}}, {{"encod0", "00f1"}, {"encod1", "00a2"}, {"encod2", "00b3"}, {"encod3", "00c4"}});
    FrameParser parser2("abcd", {{"00f1", "encod0"}, {"00a2", "encod1"}, {"00b3", "encod2"}, {"00c4", "encod3"}}, {{"0011", "test0"}, {"00a1", "test1"},{"0044", "test2"},{"00ce", "id3"}});*/
    let testFrame= "abcd26001102000a00a1044020000000440440b6666600ce0c4144cccd4205333342913333a8";
    parser.parse_frame(testFrame.to_string());
    let encoded_frame = parser.encode_frame(HashMap::from([("encod0".to_string(), vec![1.2, 2.1, 3.6]), 
                                                                                 ("encod1".to_string(), vec![4.5, 5.9]), 
                                                                                 ("encod2".to_string(), vec![6.7, 7.8, 8.9]), 
                                                                                 ("encod3".to_string(), vec![9.0])]));
    println!("Encoded Frame: {}", encoded_frame);
    let parsed_data = parser2.parse_frame(encoded_frame);
    println!("Parsed Data: ");
    for (id, values) in parsed_data {
        print!("  {}: ", id);
        for value in values {
            print!("{} ", value);
        }
        println!();
    }
    /*let encodString= parser.encodeFrame({
        {"encod0", {1.2f, 2.1f, 3.6f}},
        {"encod1", {4.5f, 5.9f}},
        {"encod2", {6.7f, 7.8f, 8.9f}},
        {"encod3", {9.0f}}
    });
    cout << "Encoded Frame: " << encodString << endl;
    map<string, vector<float>> parsedData = parser2.parseFrame(encodString);
    cout << "Parsed Data: " << endl;
    for (const auto& [id, values] : parsedData) {
        cout << "  " << id << ": ";
        for (float value : values) {
            cout << value << " ";
        }
        cout << endl;
    }*/



}

/*fn main()  -> Result<(), slint::PlatformError> {
    let mut soft = OsprAiSoftware::new();
    return soft.run();
}*/