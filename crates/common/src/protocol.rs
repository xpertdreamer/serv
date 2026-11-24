use std::{fmt::Write, io::{Read, Write as IoWrite}, net::TcpStream, num::ParseFloatError};

const SEPARATOR: char = '|';
const SPACE: char = ' ';

#[derive(Copy, Clone)]
enum RspKind {
    Text,
    VecFloat,
}

impl From<RspKind> for char {
    fn from(kind: RspKind) -> Self {
        match kind {
            RspKind::Text => 'T',
            RspKind::VecFloat => 'F',
        }
    }
}

impl TryFrom<char> for RspKind {
    type Error = ();

    fn try_from(c: char) -> Result<Self, Self::Error> {
        match c {
            'T' => Ok(RspKind::Text),
            'F' => Ok(RspKind::VecFloat),
            _ => Err(()),
        }
    }
}

pub fn is_kind(c: char) -> bool {
    RspKind::try_from(c).is_ok()
}

pub struct Message {
    kind: RspKind,
    body_len: usize,
    pub body: String,
}

impl Message {
    pub fn new() -> Self {
        Self { kind: RspKind::Text, body_len: 0, body: String::new() }
    }

    pub fn build(&self) -> String {
        format!("{}{}{}{}{}",
                char::from(self.kind),
                SEPARATOR,
                self.body_len,
                SEPARATOR,
                self.body)
    }

    pub fn parse(raw: &str) -> Option<Self> {
        let mut chars = raw.chars();
        let first = chars.next()?;
        let kind = RspKind::try_from(first).ok()?;

        if chars.next()? != SEPARATOR {
            return None;
        }

        let rest = chars.as_str();
        let (len, body) = rest.split_once(SEPARATOR)?;
        let body_len = len.parse::<usize>().ok()?;
        if body_len == body.len() {
            Some (Self {
                kind,
                body_len,
                body: body.to_string()
            })
        } else {
            None
        }
    }

    pub fn with_text(t: impl Into<String>) -> Self {
        let body = t.into();
        Self {
            kind: RspKind::Text,
            body_len: body.len(),
            body: body,
        }
    }

    pub fn with_vec(vector: &[f32]) -> Self {
        let mut s = String::with_capacity(vector.len() * 12);
        for (i, x) in vector.iter().enumerate() {
            if i != 0 {
                s.push(SPACE);
            }
            let _ = write!(s, "{}", x);
        }
        Self {
            kind: RspKind::VecFloat,
            body_len: vector.len(),
            body: s
        }
    }

    pub fn as_vec(&self) -> Result<Vec<f32>, ParseFloatError> {
        let mut res = Vec::with_capacity(self.body_len);
        for token in self.body.split(SPACE) {
            if !token.is_empty() {
                res.push(token.parse::<f32>()?);
            }
        }
        Ok(res)
    }
}

fn send_ueof(stream: &mut TcpStream, buffer: &[u8]) -> Result<(), String> {
    stream.write_all(buffer).map_err(|x| x.to_string())
}

pub fn send_msg(stream: &mut TcpStream, m: &Message) -> bool {
    let packet = m.build();
    let Ok(len) = u32::try_from(packet.len()) else {
        return false;
    };
    send_ueof(stream, &len.to_be_bytes()).is_ok()
        && send_ueof(stream, packet.as_bytes()).is_ok()
}

fn recv_ueof(stream: &mut TcpStream, buf: &mut [u8]) -> Result<(), String> {
    stream.read_exact(buf).map_err(|x| x.to_string())
}

pub fn recv_msg(stream: &mut TcpStream, msg: &mut Message) -> bool {
    let mut len_bytes = [0u8; 4];
    if recv_ueof(stream, &mut len_bytes).is_err() {
        return false;
    }
    let len = u32::from_be_bytes(len_bytes) as usize;

    let mut buf = vec![0u8; len];
    if recv_ueof(stream, &mut buf).is_err() {
        return false;
    }

    let Ok(result) = String::from_utf8(buf) else {
        return false;
    };

    if let Some(parsed) = Message::parse(&result) {
        *msg = parsed;
        true
    } else {
        false
    }
}
