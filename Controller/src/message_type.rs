pub enum MessageType {
    Text(String),
    Binary(Vec<u8>),
}