// TODO: logging

use std::net::TcpListener;
use std::io::{Error, ErrorKind};

use std::net::SocketAddr;
use socket2::{Domain, Protocol, Socket, Type};

use shell::sshell::*;
use common::{protocol::Message, util::error};
use common::protocol::recv_msg;

const SERVER_ADDRESS: &str = "127.0.0.1:4321";
const BACKLOG: i32 = 1;


fn reuse(addr: SocketAddr) -> std::io::Result<TcpListener> {
    let domain = if addr.is_ipv6() { Domain::IPV6 } else { Domain::IPV4 };
    let socket = Socket::new(domain, Type::STREAM, Some(Protocol::TCP))?;

    socket.set_reuse_address(true)?;
    socket.bind(&addr.into())?;
    socket.listen(BACKLOG)?;
    Ok(socket.into())
}

fn help_handler() -> String {
    String::from("Help:\n\thelp - show this message")
}

fn main() -> std::io::Result<()> {
    // TODO: unwrap?
    let addr: SocketAddr = SERVER_ADDRESS.parse().map_err(|e| Error::new(ErrorKind::InvalidInput, e))?;
    let listener = reuse(addr);
    let (mut sock, _) = listener?.accept()?;

    let cmds = vec![
        Command {
            name: String::from("help"),
            handler: help_handler,
        },
    ];

    let mut shell = match ServerShell::create(sock.try_clone()?, cmds) {
        Some(v) => v,
        None => {
            panic!("{}", error("error occured while creating shell"));
        }
    };
    loop {
        let mut request = Message::new();
        if !recv_msg(&mut sock, &mut request) {
            break;
        }
        shell.execute(request.body);
    }

    Ok(())
}
