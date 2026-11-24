use std::net::TcpStream;
use shell::cshell::*;

fn print_usage() {
    eprintln!("usage: client <host address> <port>")
}

fn main() -> std::io::Result<()> {
    let mut args = std::env::args().skip(1);
    let host = match args.next() {
        Some(a) => a,
        None => {
            print_usage();
            std::process::exit(1);
        }
    };
    let port = match args.next() {
        Some(p) => p,
        None => {
            print_usage();
            std::process::exit(1);
        }
    };
    let sock = TcpStream::connect(format!("{host}:{port}"))?;
    println!("connected to {host}:{port}");

    let mut shell = ClientShell::create(sock);
    while shell.daemon() {
        // pass
    }

    Ok(())
}
