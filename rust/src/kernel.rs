use std::time::{Duration, Instant};

pub const MAX_PROCESSES: usize = 16;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ProcessState {
    Running,
    Ready,
    Blocked,
    Sleeping,
    Terminated,
}

impl ProcessState {
    pub fn short(self) -> &'static str {
        match self {
            Self::Running => "RUN",
            Self::Ready => "RDY",
            Self::Blocked => "BLK",
            Self::Sleeping => "SLP",
            Self::Terminated => "END",
        }
    }
}

#[derive(Debug, Clone)]
pub struct Process {
    pub pid: u32,
    pub name: String,
    pub state: ProcessState,
    pub priority: u8,
    pub stack_size: usize,
    created_at: Instant,
}

impl Process {
    pub fn uptime(&self) -> Duration {
        self.created_at.elapsed()
    }
}

#[derive(Debug)]
pub struct Kernel {
    booted_at: Instant,
    next_pid: u32,
    processes: Vec<Process>,
    messages: Vec<String>,
}

impl Default for Kernel {
    fn default() -> Self {
        Self::new()
    }
}

impl Kernel {
    pub fn new() -> Self {
        let mut kernel = Self {
            booted_at: Instant::now(),
            next_pid: 1,
            processes: Vec::with_capacity(MAX_PROCESSES),
            messages: Vec::new(),
        };
        kernel.log("[KERNEL] Kernel initialized");
        kernel
    }

    pub fn create_process(
        &mut self,
        name: impl Into<String>,
        stack_size: usize,
        priority: u8,
    ) -> Result<u32, String> {
        if self.processes.len() >= MAX_PROCESSES {
            self.log("[KERNEL] ERROR: Process table full");
            return Err("process table full".to_string());
        }

        let pid = self.next_pid;
        self.next_pid += 1;
        let name = name.into();
        self.processes.push(Process {
            pid,
            name: name.clone(),
            state: ProcessState::Ready,
            priority,
            stack_size,
            created_at: Instant::now(),
        });
        self.log(format!(
            "[KERNEL] Created process '{name}' (PID: {pid}, Priority: {priority})"
        ));
        Ok(pid)
    }

    pub fn kill_process(&mut self, pid: u32) -> Result<(), String> {
        let index = self.processes.iter().position(|process| process.pid == pid);
        match index {
            Some(index) => {
                let process = self.processes.remove(index);
                self.log(format!(
                    "[KERNEL] Killed process '{}' (PID: {})",
                    process.name, process.pid
                ));
                Ok(())
            }
            None => {
                self.log(format!("[KERNEL] ERROR: Process PID {pid} not found"));
                Err(format!("process PID {pid} not found"))
            }
        }
    }

    pub fn list_processes(&self) -> Vec<String> {
        self.processes
            .iter()
            .map(|process| {
                format!(
                    "{}: {:<12} P:{} {} {}s",
                    process.pid,
                    process.name,
                    process.priority,
                    process.state.short(),
                    process.uptime().as_secs()
                )
            })
            .collect()
    }

    pub fn process_state(&self, pid: u32) -> ProcessState {
        self.processes
            .iter()
            .find(|process| process.pid == pid)
            .map(|process| process.state)
            .unwrap_or(ProcessState::Terminated)
    }

    pub fn uptime(&self) -> Duration {
        self.booted_at.elapsed()
    }

    pub fn process_count(&self) -> usize {
        self.processes.len()
    }

    pub fn messages(&self) -> &[String] {
        &self.messages
    }

    pub fn log(&mut self, message: impl Into<String>) {
        const MAX_LOG: usize = 100;
        if self.messages.len() >= MAX_LOG {
            self.messages.remove(0);
        }
        self.messages.push(message.into());
    }
}
