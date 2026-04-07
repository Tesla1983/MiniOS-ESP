#include "sysinfo.h"
#include "display.h"
#include "config.h"
#include <esp_system.h>
#include <WiFi.h>

void showMem() {
    uint32_t fbytes = ESP.getFreeHeap();
    uint32_t mibytes = ESP.getMinFreeHeap();
    uint32_t mabytes = ESP.getMaxAllocHeap();
    float fkb = fbytes / 1024.0;
    float mikb = mibytes / 1024.0;
    float makb = mabytes / 1024.0;

    char buf[100];
    sprintf(buf, "Free Heap: %u bytes (%.2f KB)", ESP.getFreeHeap(), fkb);
    printLine(buf);
    sprintf(buf, "Min Free Heap: %u bytes (%.2f KB)", ESP.getMinFreeHeap(), mikb);
    printLine(buf);
    sprintf(buf, "Max Alloc Heap: %u bytes (%.2f KB)", ESP.getMaxAllocHeap(), makb);
    printLine(buf);
}

void showUptime() {
    unsigned long s = millis() / 1000;
    unsigned long h = s / 3600;
    unsigned long m = (s % 3600) / 60;
    unsigned long sec = s % 60;
    printLine("Uptime: " + std::to_string(h) + "h " + std::to_string(m) + "m " + std::to_string(sec) + "s");
}

void doReboot() {
    printLine("Rebooting...");
    vTaskDelay(100 / portTICK_PERIOD_MS);
    ESP.restart();
}

void showChipInfo() {
    printLine("Chip Model: " + std::string(ESP.getChipModel()));
    printLine("Chip Cores: " + std::to_string(ESP.getChipCores()));
    printLine("Chip Revision: " + std::to_string(ESP.getChipRevision()));
}

void showCPUInfo() {
    printLine(
        "CPU: " +
        std::to_string(ESP.getCpuFreqMHz()) + " MHz"
    );
}

void showFlashInfo() {
    uint32_t flashSize = ESP.getFlashChipSize() / 1024 / 1024;
    printLine(
        "Flash: " +
        std::to_string(flashSize) + " MB"
    );
    printLine("Flash Speed: " + std::to_string(ESP.getFlashChipSpeed() / 1000000) + " MHz");
}

void showWiFiInfo() {
    if (WiFi.isConnected()) {
        printLine("WiFi RSSI: " + std::to_string(WiFi.RSSI()) + " dBm");
        printLine("WiFi Channel: " + std::to_string(WiFi.channel()));
        printLine("MAC: " + std::string(WiFi.macAddress().c_str()));
    } else {
        printLine("WiFi: Disconnected");
    }
}

