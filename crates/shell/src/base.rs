use std::io;

pub const DEFAULT_PROMPT: &str = ">";
pub const EMPTY: usize = 0;

pub enum InputError {
    IoError(io::Error),
    Empty,
}
