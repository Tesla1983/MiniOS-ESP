# MiniOS-ESP 中文文档

[English README](README.md)

MiniOS-ESP 是一个面向 ESP32 微控制器的轻量级、类 Unix 命令行操作系统，支持 ST7789 TFT 显示屏。项目提供串口交互式 Shell、TFT 文本显示、SPIFFS 文件系统、WiFi 网络工具、时间同步、计算器、主题、屏保和基于 FreeRTOS 的进程管理能力。

> 当前版本：**MiniOS-ESP v2.0.4**

## 目录

1. [项目概览](#项目概览)
2. [主要特性](#主要特性)
3. [Rust 重写版本](#rust-重写版本)
4. [系统架构](#系统架构)
5. [硬件准备](#硬件准备)
6. [构建与烧录](#构建与烧录)
7. [命令速查](#命令速查)
8. [核心模块](#核心模块)
9. [故障排查](#故障排查)
10. [开发说明](#开发说明)

---

## 项目概览

MiniOS-ESP 的目标是在资源受限的 ESP32 上提供一个小型、可交互、可扩展的命令行环境。用户可以通过串口终端输入命令，同时在 ST7789 屏幕上查看输出。系统底层使用 FreeRTOS 任务作为进程模型，并将显示、文件、网络、时间和应用命令拆分成多个模块，便于维护和扩展。

适用场景包括：

- ESP32 命令行实验环境
- 小型嵌入式工具箱
- TFT 显示和串口 Shell 示例项目
- FreeRTOS 进程/任务管理学习项目
- 便携式网络诊断与文件管理演示

## 主要特性

- **FreeRTOS 内核风格进程管理**：提供进程创建、终止、列表和系统状态查看。
- **SPIFFS 文件系统**：支持写入、追加、读取、删除、重命名、复制和列出文件。
- **WiFi 与网络工具**：支持 WiFi 连接、扫描、网络信息、HTTP GET 和 ping。
- **NTP 时间同步**：提供时间显示、日历、计时器、秒表和闹钟。
- **计算器与绘图**：支持数学表达式求值和图形化相关工具。
- **TFT 显示输出**：通过 ST7789 显示命令输出、Logo、屏保和主题色彩。
- **串口交互 Shell**：支持类似 Unix 的命令别名和历史记录。
- **Rust 主机端重写**：仓库包含 Rust 版 Shell 核心，可在桌面环境中运行和测试。

## Rust 重写版本

仓库根目录包含 `Cargo.toml`，并在 `rust/src/` 下提供一个 Rust 版本的 MiniOS Shell 核心。该版本当前作为主机端 CLI 运行，便于在没有 ESP32 硬件的情况下开发和测试命令解析、文件命令、主题、显示缓冲和进程表逻辑。

运行 Rust 版本：

```bash
cargo run --bin minios
```

运行 Rust 测试：

```bash
cargo test
```

Rust 版本包含的核心模块：

| 模块 | 说明 |
| --- | --- |
| `rust/src/shell.rs` | 命令解析、命令分发、历史记录和主机端 REPL 逻辑 |
| `rust/src/kernel.rs` | 进程表、PID、进程状态、内核日志和系统运行时间 |
| `rust/src/display.rs` | 文本显示缓冲区、滚动历史和主题状态 |
| `rust/src/filesystem.rs` | 文件系统 trait 与内存文件系统实现 |
| `rust/src/theme.rs` | 主题名称解析和 RGB565 颜色定义 |

> 注意：Rust 主机端版本中的 WiFi、ping、curl 等硬件/网络相关命令目前是安全占位实现。ESP32 硬件后端可在后续基于 Rust HAL 或绑定继续补齐。

## 系统架构

```text
┌─────────────────────────────────────────┐
│             用户界面层                  │
│        串口终端 + TFT 显示屏            │
└─────────────────┬───────────────────────┘
                  │
┌─────────────────▼───────────────────────┐
│             命令 Shell 层               │
│        命令解析、历史记录、别名          │
└─────────────────┬───────────────────────┘
                  │
┌─────────────────▼───────────────────────┐
│             应用服务层                  │
│  文件系统 / 网络 / 时间 / 计算器 / 主题  │
└─────────────────┬───────────────────────┘
                  │
┌─────────────────▼───────────────────────┐
│             内核层 FreeRTOS             │
│        任务调度、进程表、系统状态        │
└─────────────────┬───────────────────────┘
                  │
┌─────────────────▼───────────────────────┐
│             硬件抽象层                  │
│        ESP32 HAL / 驱动 / ST7789         │
└─────────────────────────────────────────┘
```

核心进程模型：

| 进程 | 优先级 | 栈大小 | 说明 |
| --- | --- | --- | --- |
| `init` | 1 | 4096 | 系统初始化 |
| `shell` | 2 | 16384 | 命令解释器 |
| `alarm` | 1 | 1024 | 时间闹钟任务 |
| `watchdog` | 0 | 1024 | 系统监控 |
| `scheduler` | 3 | 2048 | 进程状态管理 |

## 硬件准备

### 必需硬件

- ESP32 开发板，例如 ESP32-WROOM-32 或兼容板
- ST7789 TFT 显示屏，分辨率 240×320
- USB 数据线，用于供电、烧录和串口通信
- 稳定电源，通常使用 USB 5V 或开发板上的 3.3V 稳压输出

### 引脚连接

| 信号 | ESP32 GPIO | 说明 |
| --- | --- | --- |
| TFT_CS | GPIO 5 | 片选 |
| TFT_DC | GPIO 2 | 数据/命令选择 |
| TFT_RST | GPIO 4 | 复位 |
| TFT_SCK | GPIO 18 | SPI 时钟 |
| TFT_SDI | GPIO 23 | SPI MOSI |
| TFT_BLK | 3.3V | 背光常亮 |
| VCC | 3.3V | 显示屏电源 |
| GND | GND | 地线 |

### 接线建议

1. SPI 线尽量短，建议小于 15cm，以减少显示异常。
2. 显示屏 VCC 和 GND 附近可并联一个 10µF 电容提高稳定性。
3. ESP32 与显示屏必须共地。
4. 固定好显示屏与杜邦线，避免移动时接触不良。

## 构建与烧录

### C++/ESP32 固件

项目使用 PlatformIO 管理 ESP32 固件。常用命令如下：

```bash
platformio run
platformio run --target upload
platformio device monitor
```

也可以使用仓库中的构建脚本：

```bash
./build.sh
```

### Rust 主机端 CLI

Rust 版本不需要 ESP32 硬件，可直接在桌面环境运行：

```bash
cargo run --bin minios
```

格式化与测试：

```bash
cargo fmt --check
cargo test
```

## 命令速查

| 命令 | 说明 | 示例 |
| --- | --- | --- |
| `help` | 显示帮助信息 | `help` |
| `version` / `ver` | 显示系统版本 | `version` |
| `clear` / `cls` | 清屏 | `clear` |
| `history` / `hist` | 显示命令历史 | `history` |
| `ls` / `dir` | 列出文件 | `ls` |
| `write` | 写入文件 | `write notes hello` |
| `append` | 追加文件 | `append notes world` |
| `read` | 读取文件 | `read notes` |
| `delete` / `rm` | 删除文件 | `rm notes` |
| `mv` / `rename` | 重命名文件 | `mv old new` |
| `cp` / `copy` | 复制文件 | `cp src dst` |
| `wifi` | 连接或配置 WiFi | `wifi` / `wifi config` |
| `scanwifi` / `wifiscan` | 扫描 WiFi | `scanwifi` |
| `ifconfig` / `netinfo` | 显示网络信息 | `ifconfig` |
| `curl` | HTTP GET 请求 | `curl https://example.com` |
| `ping` | ping 主机 | `ping example.com 4` |
| `ps` / `processes` / `top` | 显示进程列表 | `ps` |
| `sysstat` / `stat` | 显示系统状态 | `stat` |
| `kill` | 结束指定 PID | `kill 3` |
| `theme` | 查看或切换主题 | `theme matrix` |
| `os` / `logo` | 显示 MiniOS Logo | `logo` |

## 核心模块

### Kernel

`kernel.cpp` 管理系统初始化、进程表、PID、任务状态和系统统计。常用能力包括创建进程、结束进程、列出进程和记录内核日志。

### Display

`display.cpp` 封装 ST7789 初始化、文本输出、清屏、滚动缓冲、Logo 和屏保渲染。屏保包含波浪、彩虹、螺旋、Matrix、火焰、星空和隧道等实验模式。

### Filesystem

`filesystem.cpp` 基于 SPIFFS 提供持久化文件存储。升级旧版本系统时，如果分区布局发生变化，可能需要格式化 SPIFFS 或执行完整擦除。

### Network

`network.cpp` 管理 WiFi 连接、扫描、网络信息、HTTP 请求和 ping。网络请求带有超时、重定向和二进制内容检测等基础保护。

### Time Utilities

`timeutils.cpp` 负责 NTP 同步、当前时间、日历、倒计时、秒表和闹钟。默认 NTP 服务器为 `pool.ntp.org`。

### Commands and Apps

`commands.cpp` 负责 Shell 命令分发，并连接计算器、图形工具、小游戏和系统工具等应用模块。

## 故障排查

### 显示屏无输出

- 检查 ST7789 的 CS、DC、RST、SCK 和 SDA/MOSI 是否连接到正确 GPIO。
- 确认显示屏供电为 3.3V，且 GND 与 ESP32 共地。
- 尝试缩短 SPI 连接线。
- 检查 `platformio.ini` 与源码中的引脚配置是否一致。

### 串口没有响应

- 确认 PlatformIO 串口监视器连接到正确端口。
- 检查波特率是否与固件设置一致。
- 烧录后按下开发板复位键。
- 确认 USB 数据线支持数据传输，而不仅是充电线。

### 文件读写失败

- 检查 SPIFFS 是否正确挂载。
- 首次烧录或分区调整后可尝试格式化 SPIFFS。
- 避免过长文件名和非法路径。

### WiFi 连接失败

- 确认 SSID 和密码正确。
- ESP32 通常只支持 2.4GHz WiFi。
- 检查路由器信号强度和 DHCP 是否可用。
- 使用 `scanwifi` 确认设备能扫描到目标网络。

## 开发说明

- C++ 固件源码位于 `src/` 和 `include/`。
- 第三方/本地显示驱动位于 `lib/`。
- Rust 重写版本位于 `rust/src/`。
- 文档和补丁说明位于 `docs/`。
- 新增命令时建议同时更新英文 README 和本中文 README。

欢迎提交 issue、PR 或实验性应用模块。对于硬件相关改动，请在 PR 中说明开发板型号、显示屏型号、接线方式和测试命令。
