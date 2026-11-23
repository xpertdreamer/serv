use std::{io::{self, BufRead, BufReader, Write}, net::TcpStream};

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
    let mut sock = TcpStream::connect(format!("{host}:{port}"))?;
    println!("connected to {host}:{port}");

    let mut reader = BufReader::new(sock.try_clone()?);
    let mut line = String::new();
    let mut response = String::new();
    let stdin = io::stdin();

    loop {
        line.clear();
        if stdin.lock().read_line(&mut line)? == 0 { break; }
        if line.trim().is_empty() { continue;  }
        sock.write_all(line.as_bytes())?;

        response.clear();
        if reader.read_line(&mut response)? == 0 {
            println!("closed");
            break;
        }
        print!("{response}");
    }

    Ok(())
}
