use std::io;

pub const DEFAULT_PROMPT : &str     = ">";
pub const EMPTY          : usize    = 0;

pub const SAMPLE_RATE    : u32      = 44_100;
pub const CHANNELS       : u16      = 1;


pub enum InputError {
    IoError(io::Error),
    Empty,
}
