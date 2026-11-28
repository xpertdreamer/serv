use std::{io::{self, ErrorKind, Write}, net::TcpStream, num::NonZero};
use common::protocol::*;
use common::util::*;
use rodio::{DeviceSinkBuilder, Player, buffer::SamplesBuffer, cpal::SAMPLE_RATE_CD};
use crate::base::*;

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

// NOTE: lifetime annotations sucks
pub struct ClientShell {
    prompt: String,
    stream: TcpStream,
    sm_player: Player
}

impl ClientShell {
    pub fn create(stream: TcpStream) -> Self {
        let sink = DeviceSinkBuilder::open_default_sink()
            .expect("default audio stream failed");
        Self { prompt: DEFAULT_PROMPT.into(), stream: stream, sm_player: Player::connect_new(sink.mixer())}
    }

    pub fn change_prompt(&mut self, prompt: impl Into<String>) {
        self.prompt = prompt.into();
    }

    fn on_vec(&mut self, vec: Vec<f32>){
        println!("recieved vector with len {}", vec.len());
        // for v in vec.iter() {
        //     print!("{} ", v);
        // }
        let source = SamplesBuffer::new(
            NonZero::new(CHANNELS).unwrap(),
            NonZero::new(SAMPLE_RATE_CD).unwrap(),
            vec);
        self.sm_player.append(source);
        // player.sleep_until_end();
    }

    pub fn exchange(&mut self, name: impl Into<String>) {
        let request: Message = Message::with_text(name);
        if !send_msg(&mut self.stream, &request) {
            eprintln!("{}", error("failed to send data"));
            return;
        }
        let mut answer: Message = Message::default();
        if !recv_msg(&mut self.stream, &mut answer) {
            eprintln!("{}", error("failed to recieve some bytes"));
            return;
        }

        match answer.kind() {
            RspKind::Text => {
                println!("{}", answer.body);
                #[allow(unused)]
                io::stdout().flush();
            },
            RspKind::VecFloat => match answer.as_vec() {
                Ok(o) => self.on_vec(o),
                Err(e) => eprintln!("{}", error(format!("bad: {}", e))),
            }
        }
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
