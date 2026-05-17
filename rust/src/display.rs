use crate::theme::Theme;

const DEFAULT_SCROLLBACK: usize = 128;

/// Text display abstraction that mirrors the TFT line buffer used by MiniOS.
#[derive(Debug, Clone)]
pub struct Display {
    lines: Vec<String>,
    scrollback: usize,
    theme: Theme,
}

impl Default for Display {
    fn default() -> Self {
        Self::new(DEFAULT_SCROLLBACK, Theme::classic())
    }
}

impl Display {
    pub fn new(scrollback: usize, theme: Theme) -> Self {
        Self {
            lines: Vec::new(),
            scrollback: scrollback.max(1),
            theme,
        }
    }

    pub fn print_line(&mut self, line: impl Into<String>) {
        self.lines.push(line.into());
        if self.lines.len() > self.scrollback {
            let overflow = self.lines.len() - self.scrollback;
            self.lines.drain(0..overflow);
        }
    }

    pub fn clear(&mut self) {
        self.lines.clear();
    }

    pub fn set_theme(&mut self, theme: Theme) {
        self.theme = theme;
    }

    pub fn theme(&self) -> Theme {
        self.theme
    }

    pub fn lines(&self) -> &[String] {
        &self.lines
    }

    pub fn take_recent(&self, count: usize) -> Vec<String> {
        let start = self.lines.len().saturating_sub(count);
        self.lines[start..].to_vec()
    }
}
