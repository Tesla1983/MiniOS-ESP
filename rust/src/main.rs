use std::io::{self, Write};

use minios_esp_rs::{CommandOutcome, MiniOs};

fn main() -> io::Result<()> {
    let mut os = MiniOs::default();
    for line in os.display.lines() {
        println!("{line}");
    }

    let stdin = io::stdin();
    loop {
        print!("{}", os.prompt());
        io::stdout().flush()?;

        let mut input = String::new();
        if stdin.read_line(&mut input)? == 0 {
            break;
        }

        let before = os.display.lines().len();
        if os.run_command(&input) == CommandOutcome::Exit {
            println!("Bye.");
            break;
        }

        for line in &os.display.lines()[before..] {
            println!("{line}");
        }
    }

    Ok(())
}
