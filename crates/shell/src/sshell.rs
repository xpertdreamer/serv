use std::net::TcpStream;
use common::{protocol::{Message, send_msg}, util::error};

pub type Handler = fn() -> String;

pub struct Command {
    pub name: String,
    pub handler: Handler,
}

pub struct ServerShell {
    stream: TcpStream,
    commands: Vec<Command>
}

impl ServerShell {
    pub fn create(stream: TcpStream, cmds: Vec<Command>) -> Option<Self> {
        if cmds.len() == 0 {
            eprintln!("{}", error("no commands passed"));
            return None;
        }
        Some(Self { stream: stream, commands: cmds})
    }

    pub fn execute(&mut self, name: impl AsRef<str>) {
        let target = name.as_ref();
        for cmd in &self.commands {
            if cmd.name == target {
                let result = (cmd.handler)();
                let message: Message = Message::with_text(result);
                if !send_msg(&mut self.stream, &message) {
                    eprintln!("{}", error("failed to send all data"));
                    return;
                }
            }
        }
        eprintln!("{}", error(format!("unkonwn command [{}]", target)));
    }

    // TODO: send vector
}
