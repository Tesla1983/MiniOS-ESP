#include <Arduino.h>
#include <string>
#include <vector>
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

/*!
    Set to true when a fullscreen OS task is exitted
    via vTaskDelete(). This flag lets the shell detect
    the unlock on its next tick and redraw the prompt in terminal.
*/
bool screenJustUnlocked = false;

bool screenLocked = false;
bool inputLocked = false;

struct CursorPosition {
    int16_t x;
    int16_t y;
};

std::vector<CursorPosition> inputPositions;

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
int16_t initY = currentCursorY;
int16_t overFlownLines  = 0;
void serialInputProcess(void *parameter) {
    const TickType_t delay = 10 / portTICK_PERIOD_MS;
    bool promptPrinted = false;
    vTaskDelay(500 / portTICK_PERIOD_MS);
    tft.setCursor(5, tft.getCursorY());
    print(">" + getDeviceName() + "@Mini:");
    promptPrinted = true;

    for (;;) {

        if (screenJustUnlocked) {
        screenJustUnlocked = false;
        input = "";
        inputPositions.clear();
        overFlownLines = 0;
        initY = currentCursorY;
        tft.setCursor(5, currentCursorY);
        print(">" + getDeviceName() + "@Mini:");
        }
        if (!screenLocked && !inputLocked && Serial.available()) {

            if (Serial.peek() == '\x1b') {
                Serial.read();
                vTaskDelay(5 / portTICK_PERIOD_MS);
                if (Serial.available() >= 2) {
                    char b = Serial.read();
                    char c = Serial.read();
                    if (b == '[') {
                        if (c == 'A') { scrollUp(4); continue; }
                        if (c == 'B') { scrollDown(4); continue; }
                        if (c == 'F') { scrollToBottom(); continue; }
                        if (c == 'H') { scrollToTop(); continue; }
                    }
                }
                continue;
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
                    tft.setCursor(5, currentCursorY);
                    print(">" + getDeviceName() + "@Mini:");
                    promptPrinted = true; 

                }
                if (input.length() > 0) {
                    
                    lineBuffer[bufferHead] = ">" + getDeviceName() + "@Mini:" + input;
                    bufferHead = (bufferHead + 1) % SCROLL_BUFFER_SIZE;
                    printLine("");
                    runCommand(input);
                    currentCursorY = tft.getCursorY();
                    tft.setCursor(5, currentCursorY);
                    print(">" + getDeviceName() + "@Mini:");
                    promptPrinted = true;

                }

                input = "";
                overFlownLines = 0;
                initY = currentCursorY;
                promptPrinted = false; 
            } else if (c == '\b' || c == 127) {
                if (input.length() > 0) {

                    input.pop_back();
                    Serial.write('\b');
                    Serial.write(' ');
                    Serial.write('\b');

                    CursorPosition previousChar = inputPositions.back();
                    inputPositions.pop_back();
                    currentCursorX = previousChar.x;
                    currentCursorY = previousChar.y;

                    tft.setCursor(currentCursorX, currentCursorY);
                    tft.print(" ");
                    tft.setCursor(currentCursorX, currentCursorY);
                }

            } else {
                input += c;
                inputPositions.push_back({currentCursorX, currentCursorY});
                print(c);
                if (initY != currentCursorY){
                    overFlownLines++;
                }
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
    createProcess(alarmCheckProcess, "alarm", 2048, 1);
    createProcess(watchdogProcess, "watchdog", 1024, 0);
    createProcess(kernelScheduler, "scheduler", 2048, KERNEL_PRIORITY);
    createProcess(serialInputProcess, "shell", 16384, 2);
}

void loop() {
    vTaskDelay(portMAX_DELAY);
}