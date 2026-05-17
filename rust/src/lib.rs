//! Rust rewrite of MiniOS-ESP's shell-oriented core.
//!
//! The original firmware targets Arduino/FreeRTOS on ESP32.  This crate keeps
//! the same MiniOS command model, process table, display buffer, theme support,
//! and filesystem verbs behind small Rust traits so it can run as a host CLI
//! today and be moved behind ESP HAL implementations later.

pub mod display;
pub mod filesystem;
pub mod games;
pub mod kernel;
pub mod shell;
pub mod theme;

pub use display::Display;
pub use filesystem::{FileSystem, MemoryFileSystem};
pub use kernel::{Kernel, Process, ProcessState};
pub use shell::{CommandOutcome, MiniOs};
pub use theme::{Theme, ThemeName};
