# MiniOS v2.1.1 | Structure & UX Improvements

## Overview
This update focuses on user experience, future-proofness, and contributability of the OS.
Adding ESP32-S3 support is a significant step forward, making it possible to connect PC-independent input
devices. The file structure has been refactored to make the codebase easier to
navigate and contribute to.

## New Features
- **ESP32-S3 support** | multi-device builds (ESP32 DevKit and ESP32-S3)
- **Ping Pong game** | playable from the terminal via `pong` command
- **D20 dice roller** | visual dice game via `d20` command

## Improvements
- Serial input process rewritten with state machine — eliminates nested blocking
  loops and reduces stack usage
- Prompt redraw logic fixed after fullscreen tasks exit (screensaver, pong)
- D20 animation and display polish

## Refactored File Structure
Headers and sources now mirror each other across three namespaces:

        .
        ├── docs
        ├── include
        │   ├── core
        │   │   ├── commands.h
        │   │   ├── config.h
        │   │   ├── display.h
        │   │   ├── filesystem.h
        │   │   ├── help.h
        │   │   ├── kernel.h
        │   │   ├── network.h
        │   │   └── theme.h
        │   ├── misc
        │   │   ├── ball.h
        │   │   ├── d20.h
        │   │   ├── grapher.h
        │   │   ├── mirror.h
        │   │   ├── pingpong.h
        │   │   ├── pug.h
        │   │   └── timeutils.h
        │   └── userspace
        │       ├── calc.h
        │       ├── fetch.h
        │       ├── network_utils.h
        │       ├── ping.h
        │       ├── sysinfo.h
        │       └── utils.h
        ├── lib
        ├── LICENSE
        ├── platformio.ini
        ├── README.md
        └── src
            ├── conf
            │   └── config.cpp
            ├── core
            │   ├── commands.cpp
            │   ├── display.cpp
            │   ├── filesystem.cpp
            │   ├── help.cpp
            │   ├── kernel.cpp
            │   ├── main.cpp
            │   ├── network.cpp
            │   └── theme.cpp
            ├── misc
            │   ├── ball.cpp
            │   ├── d20.cpp
            │   ├── grapher.cpp
            │   ├── mirror.cpp
            │   ├── pingpong.cpp
            │   ├── pug.cpp
            │   └── timeutils.cpp
            └── userspace
                ├── calc.cpp
                ├── fetch.cpp
                ├── network_utils.cpp
                ├── ping.cpp
                ├── sysinfo.cpp
                └── utils.cpp

## Repository
https://github.com/VuqarAhadli
