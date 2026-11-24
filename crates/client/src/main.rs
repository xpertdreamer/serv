use std::{env, net::TcpStream};
use shell::cshell::*;
use tracing::info;

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
    let log_dir = env::current_exe()
        .expect("failed to get current path")
        .parent()
        .expect("failed to get current path parent")
        .to_path_buf().join("log");

    let file_appender = tracing_appender::rolling::daily(log_dir, "client.log");
    tracing_subscriber::fmt()
        .with_writer(file_appender)
        .with_ansi(false)
        .init();

    let sock = TcpStream::connect(format!("{host}:{port}"))?;
    info!("connected to {host}:{port}");

    let mut shell = ClientShell::create(sock);
    while shell.daemon() {
        // pass
    }

    info!("exiting");
    Ok(())
}
