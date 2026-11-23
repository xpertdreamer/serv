// TODO: logging

use std::{io::{BufRead, BufReader, Write}, net::TcpListener};

const SERVER_ADDRESS: &str = "127.0.0.1:4321";

fn main() -> std::io::Result<()> {
    // TODO: unwrap?
    let listener = TcpListener::bind(SERVER_ADDRESS)?;

    let (mut sock, _) = listener.accept()?;
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
