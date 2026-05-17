use crate::display::Display;
use crate::filesystem::{FileSystem, MemoryFileSystem};
use crate::games;
use crate::kernel::Kernel;
use crate::theme::{Theme, ThemeName};

const HISTORY_SIZE: usize = 10;
const OS_VERSION: &str = env!("CARGO_PKG_VERSION");

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum CommandOutcome {
    Continue,
    Exit,
}

#[derive(Debug, Default, Clone, PartialEq, Eq)]
struct CommandArgs {
    cmd: String,
    arg1: String,
    arg2: String,
    rest: String,
}

impl CommandArgs {
    fn parse(input: &str) -> Self {
        let mut parts = input
            .splitn(4, char::is_whitespace)
            .filter(|part| !part.is_empty());
        Self {
            cmd: parts.next().unwrap_or_default().to_string(),
            arg1: parts.next().unwrap_or_default().to_string(),
            arg2: parts.next().unwrap_or_default().to_string(),
            rest: parts.next().unwrap_or_default().trim().to_string(),
        }
    }

    fn text_tail(&self) -> String {
        match (self.arg2.is_empty(), self.rest.is_empty()) {
            (true, _) => String::new(),
            (false, true) => self.arg2.clone(),
            (false, false) => format!("{} {}", self.arg2, self.rest),
        }
    }
}

#[derive(Debug)]
pub struct MiniOs<F: FileSystem = MemoryFileSystem> {
    pub display: Display,
    pub fs: F,
    pub kernel: Kernel,
    history: Vec<String>,
    device_name: String,
}

impl Default for MiniOs<MemoryFileSystem> {
    fn default() -> Self {
        Self::new(MemoryFileSystem::new())
    }
}

impl<F: FileSystem> MiniOs<F> {
    pub fn new(fs: F) -> Self {
        let mut kernel = Kernel::new();
        let _ = kernel.create_process("init", 4096, 1);
        let _ = kernel.create_process("shell", 16384, 2);
        let _ = kernel.create_process("watchdog", 1024, 0);
        let mut os = Self {
            display: Display::default(),
            fs,
            kernel,
            history: Vec::with_capacity(HISTORY_SIZE),
            device_name: "esp32".to_string(),
        };
        os.boot_banner();
        os
    }

    pub fn set_device_name(&mut self, name: impl Into<String>) {
        self.device_name = name.into();
    }

    pub fn prompt(&self) -> String {
        format!(">{}@Mini:", self.device_name)
    }

    pub fn run_command(&mut self, input: &str) -> CommandOutcome {
        let command = input.trim();
        if command.is_empty() {
            return CommandOutcome::Continue;
        }

        self.add_history(command);
        let args = CommandArgs::parse(command);
        let base = args.cmd.to_ascii_lowercase();

        match base.as_str() {
            "write" => self.write_command(&args),
            "append" => self.append_command(&args),
            "read" => self.read_command(&args),
            "delete" | "rm" => self.delete_command(&args),
            "ls" | "dir" => self.list_files(),
            "mv" | "rename" => self.rename_command(&args),
            "cp" | "copy" => self.copy_command(&args),
            "clear" | "cls" => self.display.clear(),
            "history" | "hist" => self.show_history(),
            "help" => self.show_help(),
            "version" | "ver" => self.show_version(),
            "os" | "logo" => self.show_logo(),
            "theme" => self.theme_command(&args),
            "ps" | "processes" | "top" => self.show_processes(),
            "sysstat" | "stat" => self.show_system_stats(),
            "kill" => self.kill_command(&args),
            "ball" => self.ball_command(&args),
            "pong" | "pingpong" => self.print_game_lines(games::pingpong_game()),
            "d20" | "dice" => self.print_game_lines(games::d20_game()),
            "coin" | "coinflip" | "flip" => self.print_game_lines(games::coin_game()),
            "echo" => self
                .display
                .print_line(command.strip_prefix(&args.cmd).unwrap_or_default().trim()),
            "reboot" | "exit" | "quit" => return CommandOutcome::Exit,
            "wifi" | "disconnect" | "scanwifi" | "wifiscan" | "ifconfig" | "netinfo"
            | "ipconfig" | "curl" | "ping" => {
                self.display.print_line(format!(
                    "{base}: hardware/network backend is not enabled in the host Rust build"
                ));
            }
            _ => self.display.print_line(format!(
                "Unknown command: {}. Type 'help' for commands.",
                args.cmd
            )),
        }

        CommandOutcome::Continue
    }

    pub fn history(&self) -> &[String] {
        &self.history
    }

    fn boot_banner(&mut self) {
        self.display.print_line("MiniOS - Rust Kernel");
        self.display.print_line("[SYSTEM] Display initialized");
        self.display.print_line("[SYSTEM] Filesystem initialized");
        self.display.print_line("[SYSTEM] MiniOS Ready");
        self.display.print_line("Type 'help' for commands");
    }

    fn add_history(&mut self, command: &str) {
        if self
            .history
            .last()
            .map(|last| last == command)
            .unwrap_or(false)
        {
            return;
        }
        if self.history.len() == HISTORY_SIZE {
            self.history.remove(0);
        }
        self.history.push(command.to_string());
    }

    fn write_command(&mut self, args: &CommandArgs) {
        if args.arg1.is_empty() || args.arg2.is_empty() {
            self.display.print_line("Usage: write <filename> <text>");
            return;
        }
        match self.fs.write_file(&args.arg1, &args.text_tail()) {
            Ok(()) => self.display.print_line(format!("Wrote {}", args.arg1)),
            Err(error) => self.display.print_line(format!("write failed: {error}")),
        }
    }

    fn append_command(&mut self, args: &CommandArgs) {
        if args.arg1.is_empty() || args.arg2.is_empty() {
            self.display.print_line("Usage: append <filename> <text>");
            return;
        }
        match self.fs.append_file(&args.arg1, &args.text_tail()) {
            Ok(()) => self.display.print_line(format!("Appended {}", args.arg1)),
            Err(error) => self.display.print_line(format!("append failed: {error}")),
        }
    }

    fn read_command(&mut self, args: &CommandArgs) {
        if args.arg1.is_empty() {
            self.display.print_line("Usage: read <filename>");
            return;
        }
        match self.fs.read_file(&args.arg1) {
            Ok(contents) => self.display.print_line(contents),
            Err(error) => self.display.print_line(format!("read failed: {error}")),
        }
    }

    fn delete_command(&mut self, args: &CommandArgs) {
        if args.arg1.is_empty() {
            self.display.print_line("Usage: delete <filename>");
            return;
        }
        match self.fs.delete_file(&args.arg1) {
            Ok(()) => self.display.print_line(format!("Deleted {}", args.arg1)),
            Err(error) => self.display.print_line(format!("delete failed: {error}")),
        }
    }

    fn rename_command(&mut self, args: &CommandArgs) {
        if args.arg1.is_empty() || args.arg2.is_empty() {
            self.display.print_line("Usage: mv <old> <new>");
            return;
        }
        match self.fs.rename_file(&args.arg1, &args.arg2) {
            Ok(()) => self
                .display
                .print_line(format!("Renamed {} to {}", args.arg1, args.arg2)),
            Err(error) => self.display.print_line(format!("rename failed: {error}")),
        }
    }

    fn copy_command(&mut self, args: &CommandArgs) {
        if args.arg1.is_empty() || args.arg2.is_empty() {
            self.display.print_line("Usage: cp <src> <dst>");
            return;
        }
        match self.fs.copy_file(&args.arg1, &args.arg2) {
            Ok(()) => self
                .display
                .print_line(format!("Copied {} to {}", args.arg1, args.arg2)),
            Err(error) => self.display.print_line(format!("copy failed: {error}")),
        }
    }

    fn list_files(&mut self) {
        let files = self.fs.list_files();
        if files.is_empty() {
            self.display.print_line("No files found.");
            return;
        }
        self.display.print_line("FILES");
        for (name, size) in files {
            self.display.print_line(format!("{name:<24} {size} bytes"));
        }
    }

    fn show_history(&mut self) {
        if self.history.is_empty() {
            self.display.print_line("No command history.");
            return;
        }
        self.display.print_line("Command history:");
        for (index, command) in self.history.iter().enumerate() {
            self.display.print_line(format!("{}: {command}", index + 1));
        }
    }

    fn show_help(&mut self) {
        self.display.print_line("Commands: help, version, clear, history, ls, write, append, read, rm, mv, cp, theme, ps, stat, kill, echo, exit");
        self.display
            .print_line("Games: ball [radius] [count] [trail], pong, d20, coin");
        self.display.print_line(
            "Network commands are compiled as hardware-backend stubs in this host build.",
        );
    }

    fn show_version(&mut self) {
        self.display
            .print_line(format!("MiniOS {OS_VERSION} (Rust)"));
        self.display
            .print_line("Repository: github.com/VuqarAhadli/MiniOS-ESP");
    }

    fn show_logo(&mut self) {
        self.display.print_line(" __  __ _       _  ___  ____");
        self.display.print_line(r"|  \/  (_)_ __ (_)/ _ \/ ___|");
        self.display.print_line(r"| |\/| | | '_ \| | | | \___ \");
        self.display.print_line("| |  | | | | | | | |_| |___) |");
        self.display.print_line(r"|_|  |_|_|_| |_|_|\___/|____/");
    }

    fn theme_command(&mut self, args: &CommandArgs) {
        if args.arg1.is_empty() {
            let theme = self.display.theme();
            self.display
                .print_line(format!("Current theme: {}", theme.name.label()));
            self.display
                .print_line("Available themes: classic, amber, matrix, ocean");
            return;
        }
        match ThemeName::parse(&args.arg1) {
            Some(name) => {
                self.display.set_theme(Theme::from_name(name));
                self.display
                    .print_line(format!("Theme set to {}", name.label()));
            }
            None => self
                .display
                .print_line("Unknown theme. Use: classic, amber, matrix, ocean"),
        }
    }

    fn show_processes(&mut self) {
        self.display.print_line("PROCESS LIST");
        self.display
            .print_line("----------------------------------");
        for line in self.kernel.list_processes() {
            self.display.print_line(line);
        }
        self.display
            .print_line("----------------------------------");
    }

    fn show_system_stats(&mut self) {
        self.display.print_line("SYSTEM STATS");
        self.display
            .print_line(format!("Uptime: {}s", self.kernel.uptime().as_secs()));
        self.display
            .print_line(format!("Processes: {}", self.kernel.process_count()));
        self.display
            .print_line("Memory: managed by Rust allocator in host build");
    }

    fn print_game_lines(&mut self, lines: Vec<String>) {
        for line in lines {
            self.display.print_line(line);
        }
    }

    fn ball_command(&mut self, args: &CommandArgs) {
        let radius = if args.arg1.is_empty() {
            10
        } else {
            match args.arg1.parse::<u16>() {
                Ok(radius) if radius > 0 && radius < 116 => radius,
                Ok(_) => {
                    self.display
                        .print_line("Error: ball radius must be between 1 and 115 pixels.");
                    return;
                }
                Err(_) => {
                    self.display
                        .print_line("Error: invalid radius, must be a number.");
                    return;
                }
            }
        };

        let count = if args.arg2.is_empty() {
            1
        } else {
            match args.arg2.parse::<usize>() {
                Ok(count) if count > 0 => count,
                _ => {
                    self.display
                        .print_line("Error: number of balls must be a positive integer.");
                    return;
                }
            }
        };

        let trail = args.rest.eq_ignore_ascii_case("trail");
        self.print_game_lines(games::ball_game(radius, count, trail));
    }

    fn kill_command(&mut self, args: &CommandArgs) {
        if args.arg1.is_empty() {
            self.display.print_line("Usage: kill <pid>");
            return;
        }
        let Ok(pid) = args.arg1.parse::<u32>() else {
            self.display.print_line("Invalid PID");
            return;
        };
        match self.kernel.kill_process(pid) {
            Ok(()) => self.display.print_line(format!("Killed process PID {pid}")),
            Err(error) => self.display.print_line(error),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn file_commands_round_trip() {
        let mut os = MiniOs::default();
        os.run_command("write notes hello");
        os.run_command("append notes  world");
        os.run_command("read notes");
        assert_eq!(os.display.lines().last().unwrap(), "helloworld");
    }

    #[test]
    fn duplicate_history_entries_are_ignored() {
        let mut os = MiniOs::default();
        os.run_command("version");
        os.run_command("version");
        assert_eq!(os.history(), &["version".to_string()]);
    }

    #[test]
    fn theme_command_accepts_named_theme() {
        let mut os = MiniOs::default();
        os.run_command("theme matrix");
        assert_eq!(os.display.theme().name, ThemeName::Matrix);
    }

    #[test]
    fn exit_commands_stop_repl() {
        let mut os = MiniOs::default();
        assert_eq!(os.run_command("exit"), CommandOutcome::Exit);
    }

    #[test]
    fn game_commands_are_available_in_host_build() {
        let mut os = MiniOs::default();

        os.run_command("ball 8 2 trail");
        assert!(os.display.lines().iter().any(|line| line.contains("弹球")));
        assert_eq!(
            os.display
                .lines()
                .iter()
                .filter(|line| line.starts_with("ball #"))
                .count(),
            2
        );

        os.run_command("pong");
        assert!(os.display.lines().iter().any(|line| line.contains("乒乓")));

        os.run_command("d20");
        assert!(os
            .display
            .lines()
            .iter()
            .any(|line| line.contains("二十面骰子")));

        os.run_command("coin");
        assert!(os
            .display
            .lines()
            .iter()
            .any(|line| line.contains("抛硬币")));
    }

    #[test]
    fn ball_command_validates_arguments() {
        let mut os = MiniOs::default();

        os.run_command("ball nope");
        assert_eq!(
            os.display.lines().last().unwrap(),
            "Error: invalid radius, must be a number."
        );

        os.run_command("ball 10 0");
        assert_eq!(
            os.display.lines().last().unwrap(),
            "Error: number of balls must be a positive integer."
        );
    }
}
