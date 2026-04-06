# MiniOS v2.1.0 | A major Milestone

## Core Improvements

### Scrolling

* Terminal output now supports **scrolling up and down** using the arrow buttons, so you can review previous messages easily.

### Persistent Configuration

* All user preferences are now stored in a **`config.cfg`** file.
  Example:

```
deviceName=vugart
theme=8
SSID=XXXX
PASS=YYYY
```

* If connected to a Wi-Fi network, the credentials are automatically saved in the config.
* Use the command `wifi conf` in the serial terminal to **auto-connect** to your saved network.

### Message Logging

* Added a new **`dmesg`** command for detailed system logs and analysis.

---

**Repository:** [https://github.com/VuqarAhadli](https://github.com/VuqarAhadli)
**Version:** 2.1.0


