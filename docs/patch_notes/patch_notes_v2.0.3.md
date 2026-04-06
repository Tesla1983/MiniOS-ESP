# MiniOS-ESP Documentation (v2.0.3)

MiniOS-ESP is a lightweight, Unix-like operating system for ESP32 microcontrollers. It features a FreeRTOS-based kernel, a custom shell, SPIFFS filesystem management, and an ST7789 TFT display interface.

## 1. Architecture & Core

MiniOS-ESP operates on a multi-process model where the kernel manages task priorities and memory allocation.

### The v2.0.3 Transition: `std::string`

As of version 2.0.3, the system has migrated from `arduino::String` to `std::string`. This reduces heap fragmentation and allows for more robust string manipulation using standard C++ libraries.

### Core Processes

| PID | Process | Priority | Stack (Bytes) | Role |
| --- | --- | --- | --- | --- |
| 1 | `init` | 1 | 4096 | Boot & Driver Init |
| 2 | `shell` | 2 | 16384 | Command Interpreter |
| 3 | `alarm` | 1 | 1024 | Time-based Interrupts |
| 4 | `watchdog` | 0 | 1024 | System Health |
| 5 | `scheduler` | 3 | 2048 | Process Lifecycle |

---

## 2. Hardware Setup

### Required Components

* **ESP32 DevKit** (WROOM-32)
* **ST7789 TFT Display** (240x320)
* **Wiring:**

| Signal | GPIO | Signal | GPIO |
| --- | --- | --- | --- |
| **TFT_CS** | 5 | **TFT_DC** | 2 |
| **TFT_RST** | 4 | **TFT_SCK** | 18 |
| **TFT_SDI** | 23 | **TFT_BLK** | 3.3V (or PWM) |

---

## 3. Command Reference

### System & Identity

* **`fetch`** (Aliases: `neofetch`, `fastfetch`): Displays system hardware info, memory, and OS logo.
* **`username <name>`**: (v2.0.3) Sets a local alias for your shell session.
* **`mem`** (Alias: `free`): Shows real-time heap usage.
* **`ps`** (Alias: `top`): Lists running processes and their states.
* **`uptime`**: Time elapsed since boot.
* **`reboot`** (Alias: `restart`): Soft resets the ESP32.

### Filesystem (SPIFFS)

* **`ls`** (Alias: `dir`): List files.
* **`write <file> <text>`**: Create or overwrite a file.
* **`append <file> <text>`**: Add text to the end of a file.
* **`read <file>`**: Display file contents.
* **`rm <file>`** (Alias: `delete`): Permanently remove a file.
* **`cp` / `mv**`: Copy or rename files.

### Network Utilities

* **`wifi`**: Interactive setup wizard for SSID/Password.
* **`scanwifi`** (Alias: `wifiscan`): Scans for 2.4GHz networks.
* **`ifconfig`** (Alias: `ipconfig`): Displays local IP and RSSI.
* **`ping <host>`**: ICMP echo request with Min/Avg/Max latency stats.
* **`curl [-v] <url>`**: Fetches HTTP content (v2.0.2 supports verbose mode and redirects).
* **`dns <host>`** (Alias: `nslookup`): Resolves IP addresses.

### Tools & Calculation

* **`calc <expression>`**: Advanced math (e.g., `calc sqrt(144) + sin(pi/2)`).
* **`graph <expr>`** (Alias: `plot`): Plots functions on the TFT display.
* **`base64 <encode|decode> <text>`**: Standard Base64 processing.
* **`hex` / `bin**`: Decimal conversion tools.

### Time & Display

* **`time`** (Alias: `date`): Shows current NTP-synced time.
* **`calendar`** (Alias: `cal`): Monthly view with current day highlighted.
* **`timer` / `stopwatch**`: Practical time measurement tools.
* **`theme <0-7>`**: Changes the UI color palette (Matrix, Classic, Cyber, etc.).
* **`screensaver`** (Alias: `ss`): High-FPS animations including Matrix, Fire, Starfield, and new v2.0.3 additions.

---

## 4. Development & Contribution

### Building the Project

We recommend **PlatformIO** for the best dependency management.

1. Clone the repo.
2. Ensure `platformio.ini` uses the `min_spiffs.csv` partition scheme.
3. Build and Upload.

### Coding Standards (v2.0.3)

When contributing new commands, follow the `std::string` standard:

```cpp
// Correct way to implement a command in v2.0.3
void handleNewCommand(std::string input) {
    if (input.empty()) {
        printLine("Error: No argument provided.");
        return;
    }
    // Use .c_str() for display/serial output if needed
    Serial.println(input.c_str());
}

```

### Known Issues & Troubleshooting

* **White Screen:** Check your `TFT_RST` pin; some displays require a physical pull-up.
* **SPIFFS Fail:** If migrating from v1.x, you **must** run `SPIFFS.format()` or use the "Erase Flash" tool to reset the partition table.
* **WiFi Failure:** ESP32 only supports **2.4GHz**. Ensure your router is not 5GHz-only.

---

## 5. License & Credits

* **Author:** Vuqar Ahadli
* **License:** MIT
* **Libraries:** Adafruit GFX, Adafruit ST7789, ESP32-Arduino Core.
