use std::io::{self, ErrorKind, Write};
use crate::util::error;

const DEFAULT_PROMPT: &str = ">";
const EMPTY: usize = 0;

pub enum InputError {
    IoError(io::Error),
    Empty,
}

pub fn get_input() -> Result<Option<String>, InputError>  {
    let mut buffer = String::new();

    match io::stdin().read_line(&mut buffer) {
        Err(e) if e.kind() == ErrorKind::Interrupted => Ok(None),
        Err(e) => Err(InputError::IoError(e)),
        Ok(EMPTY) => Ok(None),
        Ok(_) => {
            if buffer.ends_with('\n') {
                buffer.pop();
                if buffer.ends_with('\r') {
                    buffer.pop();
                }
            }

            if buffer.is_empty() {
                return Err(InputError::Empty);
            }

            Ok(Some(buffer))
        }
    }
}

pub struct ClientShell {
    prompt: String,
}

impl ClientShell {
    pub fn create() -> Self {
        Self { prompt: DEFAULT_PROMPT.into() }
    }

    pub fn change_prompt(&mut self, prompt: impl Into<String>) {
        self.prompt = prompt.into();
    }

    pub fn exchange(&mut self, _name: impl Into<String>) {
        eprintln!("{}", error("TODO"));
    }

    pub fn daemon(&mut self) -> bool {
        print!("\n{} ", self.prompt);
        #[allow(unused)]
        io::stdout().flush();

        match get_input() {
            Ok(None) => return false,
            Err(InputError::IoError(_)) => return false,
            Err(InputError::Empty) => return false,
            Ok(Some(input)) => {
                self.exchange(input);
                true
            }
        }
    }
}
