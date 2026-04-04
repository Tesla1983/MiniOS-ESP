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
    logKernelMessage("[SYSTEM] Display initialized");

    if (!initFilesystem()) {
        printLine("[ERROR] Filesystem failed");
        logKernelMessage("[ERROR] Filesystem failed");
        vTaskDelete(NULL);
        return;
    }
    
    printLine("[SYSTEM] Filesystem initialized");
    logKernelMessage("[SYSTEM] Filesystem initialized");
    loadConfig(); 
    setTheme(std::to_string(getSavedTheme()));  
    printLine("[SYSTEM] MiniOS Ready");
    logKernelMessage("[SYSTEM] MiniOS Ready");
    printLine("Type 'help' for commands");
    printLine("");
    
    vTaskDelay(100 / portTICK_PERIOD_MS);
    vTaskDelete(NULL);
}

void serialInputProcess(void *parameter) {
    const TickType_t delay = 10 / portTICK_PERIOD_MS;
    bool promptPrinted = false;

    while (1) {
        if (!screenLocked && !inputLocked && Serial.available()) {

            if (Serial.peek() == '\x1b') {
                Serial.read();
                vTaskDelay(5 / portTICK_PERIOD_MS);
                if (Serial.available() >= 2) {
                    char b = Serial.read();
                    char c = Serial.read();
                    if (b == '[') {
                        if (c == 'A') { scrollUp(4);      continue; }
                        if (c == 'B') { scrollDown(4);    continue; }
                        if (c == 'F') { scrollToBottom(); continue; }
                        if (c == 'H') { scrollToTop();    continue; }
                    }
                }
                continue;
            }

            if (!promptPrinted) {
                tft.setCursor(5,currentCursorY);
                print(">" + getDeviceName() + "@Mini:");
                promptPrinted = true;
            }

            char c = Serial.read();

            if (c == '\n') {
                if (screenCleared) {
                    // if (input.length() > 0) {
                    //     print(input);
                    // }
                    // printLine("");
                    currentCursorY = tft.getCursorY();
                    screenCleared = false;
                }
                if (input.length() > 0) {
                    printLine("");
                    runCommand(input);

                }
                input = "";
                promptPrinted = false; 

            } else if (c == '\b' || c == 127) {
                if (input.length() > 0) {
                    input.pop_back();
                    Serial.write('\b');
                    Serial.write(' ');
                    Serial.write('\b');
                    currentCursorX -= 6;
                    tft.setCursor(currentCursorX, currentCursorY);
                    tft.print(" ");
                    tft.setCursor(currentCursorX, currentCursorY);
                }

            } else {
                input += c;
                print(c);
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