/// Named color themes from the C++ firmware, represented as RGB565 pairs.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ThemeName {
    Classic,
    Amber,
    Matrix,
    Ocean,
}

impl ThemeName {
    pub fn parse(value: &str) -> Option<Self> {
        match value.trim().to_ascii_lowercase().as_str() {
            "0" | "classic" | "default" => Some(Self::Classic),
            "1" | "amber" => Some(Self::Amber),
            "2" | "matrix" | "green" => Some(Self::Matrix),
            "3" | "ocean" | "blue" => Some(Self::Ocean),
            _ => None,
        }
    }

    pub fn label(self) -> &'static str {
        match self {
            Self::Classic => "classic",
            Self::Amber => "amber",
            Self::Matrix => "matrix",
            Self::Ocean => "ocean",
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Theme {
    pub name: ThemeName,
    pub foreground: u16,
    pub background: u16,
}

impl Theme {
    pub const fn classic() -> Self {
        Self {
            name: ThemeName::Classic,
            foreground: 0xffff,
            background: 0x0000,
        }
    }

    pub const fn from_name(name: ThemeName) -> Self {
        match name {
            ThemeName::Classic => Self {
                name,
                foreground: 0xffff,
                background: 0x0000,
            },
            ThemeName::Amber => Self {
                name,
                foreground: 0xfd20,
                background: 0x0000,
            },
            ThemeName::Matrix => Self {
                name,
                foreground: 0x07e0,
                background: 0x0000,
            },
            ThemeName::Ocean => Self {
                name,
                foreground: 0x07ff,
                background: 0x0010,
            },
        }
    }
}
