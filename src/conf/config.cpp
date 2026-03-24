#include "config.h"
#include "display.h"
#include "kernel.h"
#include <Arduino.h>
#include <SPIFFS.h>
#include <string>


const char* NTP_SERVER = "pool.ntp.org";
const long  GMT_OFFSET = 4 * 3600;   
const int   DAYLIGHT_OFFSET = 0;
const char* OS_VERSION = "MiniOS-ESP v2.1.0";

static std::string deviceName = "Mini";
static int savedTheme = 0;


#define CONFIG_FILE "/config.cfg"

static void setConfigKey(const std::string& key, const std::string& value) {
    std::string content = "";
    File f = SPIFFS.open(CONFIG_FILE, FILE_READ);
    if (f) {
        while (f.available()) {
            String line = f.readStringUntil('\n');
            std::string l = std::string(line.c_str());
            if (!l.empty() && l.back() == '\r') l.pop_back();
            if (l.empty()) continue;

            size_t tilleq = l.find('=');
            if (tilleq != std::string::npos && l.substr(0, tilleq) == key) continue;

            content += l + "\n";
        }
        f.close();
    }

    content += key + "=" + value + "\n";

    File out = SPIFFS.open(CONFIG_FILE, FILE_WRITE);
    if (out) {
        out.print(content.c_str());
        out.flush();
        out.close();
    }
}

static std::string getConfigKey(const std::string& key) {
    File f = SPIFFS.open(CONFIG_FILE, FILE_READ);
    if (!f) return "";

    while (f.available()) {
        String line = f.readStringUntil('\n');
        std::string l = std::string(line.c_str());
        if (!l.empty() && l.back() == '\r') l.pop_back();
        if (l.empty()) continue;

        size_t eq = l.find('=');
        if (eq != std::string::npos && l.substr(0, eq) == key) {
            f.close();
            return l.substr(eq + 1);
        }
    }

    f.close();
    return "";
}


void loadConfig() {
    if (!SPIFFS.exists(CONFIG_FILE)) {
        printLine("[CONFIG] No config file, using defaults.");
        logKernelMessage("[CONFIG] No config file, using defaults.");
        return;
    }

    std::string name = getConfigKey("deviceName");
    if (!name.empty() && name.length() <= 32) {
        deviceName = name;
    }

    std::string themeStr = getConfigKey("theme");
    if (!themeStr.empty()) {
        try {
            savedTheme = std::stoi(themeStr);
        } catch (...) {
            savedTheme = 0;
        }
    }

    printLine("[CONFIG] Loaded: name=" + deviceName +
              " theme=" + std::to_string(savedTheme));
    logKernelMessage("[CONFIG] Loaded: name=" + deviceName +
                             " theme=" + std::to_string(savedTheme));
}

void saveConfig() {
    setConfigKey("deviceName", deviceName);
    setConfigKey("theme", std::to_string(savedTheme));
    printLine("[CONFIG] Saved.");
    logKernelMessage("[CONFIG] Saved.");
}


std::string getDeviceName() {
    return deviceName;
}

void setDeviceName(const std::string& name) {
    if (name.empty()) {
        printLine("Error: Username cannot be empty.");
        return;
    }
    if (name.length() > 32) {
        printLine("Error: Username too long (max 32).");
        return;
    }
    deviceName = name;
    setConfigKey("deviceName", deviceName);
    printLine("Username set: " + deviceName);
    logKernelMessage("[SYSTEM] Username set: " + deviceName);

}


int getSavedTheme() {
    return savedTheme;
}

void saveSavedTheme(int index) {
    savedTheme = index;
    setConfigKey("theme", std::to_string(savedTheme));
}

void setWifiConfig(const std::string& SSID, const std::string& PASS){
    setConfigKey("SSID", SSID);
    setConfigKey("PASS", PASS);
    printLine("[NETWORK] config updated.");
    logKernelMessage("[NETWORK] config updated.");
}


std::string getWifiSSID() {
    return getConfigKey("SSID");
}

std::string getWifiPass() {
    return getConfigKey("PASS");
}

