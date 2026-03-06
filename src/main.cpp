#include <Arduino.h>
#include <string>
#include "config.h"
#include "display.h"
#include "filesystem.h"
#include "network.h"
#include "theme.h"
#include "commands.h"
#include "pug.h"
#include "timeutils.h"
#include "kernel.h"

std::string input = "";
bool screenLocked = false;
bool inputLocked = false;

void initProcess(void *parameter) {
    printLine("MiniOS - FreeRTOS Kernel");

    printLine("[SYSTEM] Display initialized");
    
    if (!initFilesystem()) {
        printLine("[ERROR] Filesystem failed");
        vTaskDelete(NULL);
        return;
    }
    
    printLine("[SYSTEM] Filesystem initialized");
    printLine("MiniOS Ready");
    printLine("Type 'help' for commands");
    printLine("");
    
    vTaskDelay(100 / portTICK_PERIOD_MS);
    vTaskDelete(NULL);
}

void serialInputProcess(void *parameter) {
    const TickType_t delay = 10 / portTICK_PERIOD_MS;
    
    while (1) {
        if (!screenLocked && !inputLocked && Serial.available()) {
            char c = Serial.read();
            if (c == '\n') {
                if (screenCleared) {
                    if (input.length() > 0) {
                        print(input);
                    }
                    printLine("");
                    currentCursorY = tft.getCursorY();
                    screenCleared = false;
                } else {
                    printLine(">" + getDeviceName() + "@Mini:" + input);
                }
                if (input.length() > 0) {
                    runCommand(input);
                }
                input = "";
                
            } else if (c == '\b' || c == 127) {
                if (input.length() > 0) {
                    input.pop_back();
                    Serial.write('\b');
                    Serial.write(' ');
                    Serial.write('\b');
                }
            } else {
                input += c;
                Serial.write(c);
            }
        }
        vTaskDelay(delay);
    }
}

void alarmCheckProcess(void *parameter) {
    const TickType_t delay = 1000 / portTICK_PERIOD_MS;
    
    while (1) {
        if (!screenLocked) {
            checkAlarm();
        }
        vTaskDelay(delay);
    }
}

void watchdogProcess(void *parameter) {
    const TickType_t delay = 5000 / portTICK_PERIOD_MS;
    
    while (1) {
        vTaskDelay(delay);
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    while (Serial.available()) {
        Serial.read();
    }
    
    Serial.println("MiniOS - FreeRTOS Kernel");
    Serial.println("Initializing...");
    
    initDisplay();
    kernelInit();
    
    createProcess(initProcess, "init", 4096, 1);
    createProcess(serialInputProcess, "shell", 16384, 2);
    createProcess(alarmCheckProcess, "alarm", 2048, 1);
    createProcess(watchdogProcess, "watchdog", 1024, 0);
    createProcess(kernelScheduler, "scheduler", 2048, KERNEL_PRIORITY);
}

void loop() {
    vTaskDelay(portMAX_DELAY);
}