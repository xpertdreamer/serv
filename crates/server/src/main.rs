// TODO: logging

use std::{io::{self, BufRead, BufReader, Write}, net::TcpListener};
use std::net::SocketAddr;
use socket2::{Domain, Protocol, Socket, Type};

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

fn main() -> std::io::Result<()> {
    // TODO: unwrap?
    let addr: SocketAddr = SERVER_ADDRESS.parse().map_err(|e| io::Error::new(io::ErrorKind::InvalidInput, e))?;
    let listener = reuse(addr);
    let (mut sock, _) = listener?.accept()?;

    let mut reader = BufReader::new(sock.try_clone()?);
    let mut line = String::new();
    loop {
        line.clear();
        if reader.read_line(&mut line)? == 0 {
            break
        }
        let text = line.trim_end_matches(['\r', '\n']);
        let resp = format!("Accepted: {} bytes ({})\n", text.len(), text);
        sock.write_all(resp.as_bytes())?;
        sock.flush()?;
    }

    Ok(())
}
