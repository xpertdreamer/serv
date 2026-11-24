use std::fmt::Display;

// colors
#[cfg(not(target_os = "windows"))]
mod colors {
    pub const ERROR_COLOR: &str = "\x1b[1;31m"; // red
    pub const RESET_COLOR: &str = "\x1b[0m";    // reset sequence
}

#[cfg(target_os = "windows")]
mod colors {
    pub const ERROR_COLOR: &str = "";
    pub const RESET_COLOR: &str = "";
}

pub use colors::*;

//  accept any type that implements Display trait
pub fn error(msg: impl Display) -> String {
    format!("{ERROR_COLOR}{msg}{RESET_COLOR}")
}
