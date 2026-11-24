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

        let text = match self.commands.iter().find(|c| c.name == target) {
            Some(cmd) => (cmd.handler)(),
            None => {
                eprintln!("{}", error(format!("unknown command [{}]", target)));
                format!("{}", error(format!("unknown command [{}]", target)))
            }
        };

        if !send_msg(&mut self.stream, &Message::with_text(text)) {
            eprintln!("{}", error("failed to send all data"));
        }
    }

    // TODO: send vector
}
