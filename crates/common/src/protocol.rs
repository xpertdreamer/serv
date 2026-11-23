const SEPARATOR: char = '|';

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
    body: String,
}

impl Message {
    pub fn build(&self) -> String {
        format!("{}{}{}{}{}",
                char::from(self.kind),
                SEPARATOR,
                self.body_len,
                SEPARATOR,
                self.body)
    }

    fn parse(raw: &str) -> Option<Self> {
        let mut chars = raw.chars();
        let first = chars.next()?;
        let kind = RspKind::try_from(first).ok()?;

        if chars.next()? != SEPARATOR {
            return None;
        }

        let rest = chars.as_str();
        let (len, body) = rest.split_once(SEPARATOR)?;
        let body_len = len.parse::<usize>().ok()?;
        Some (Self {
            kind,
            body_len,
            body: body.to_string()
        })
    }

    fn text(t: impl Into<String>) -> Self {
        let body = t.into();
        Self {
            kind: RspKind::Text,
            body_len: 0,
            body: body,
        }
    }


}
