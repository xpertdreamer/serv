use std::net::TcpStream;
use common::{protocol::{Message, send_msg}, util::error};

pub type Handler = fn() -> String;
pub type VecHandler = fn() -> Vec<f32>;

pub struct VecCommand {
    pub name: String,
    pub handler: VecHandler,
}

pub struct Command {
    pub name: String,
    pub handler: Handler,
}

pub struct ServerShell {
    stream: TcpStream,
    commands: Vec<Command>,
    vcommands: Vec<VecCommand>,
}

impl ServerShell {
    pub fn create(stream: TcpStream, cmds: Vec<Command>, vcmds: Vec<VecCommand>) -> Option<Self> {
        if cmds.len() == 0 {
            eprintln!("{}", error("no commands passed"));
            return None;
        }
        Some(Self { stream: stream, commands: cmds, vcommands: vcmds })
    }

    pub fn execute(&mut self, name: impl AsRef<str>) {
        let target = name.as_ref();

        let msg = if let Some(c) = self.commands.iter().find(|i| i.name == target) {
            Message::with_text((c.handler)())
        } else if let Some(c) = self.vcommands.iter().find(|i| i.name == target) {
            Message::with_vec(&(c.handler)())
        } else {
            // TODO: log
            Message::with_text(format!("{}", error(format!("unknown command [{}]", target))))
        };

        if !send_msg(&mut self.stream, &msg) {
            eprintln!("{}", error("failed to send all data"));
        }
    }
}
