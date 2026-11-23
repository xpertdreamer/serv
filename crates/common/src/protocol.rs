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
