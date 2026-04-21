#include "display.h"
#include "theme.h"
#include "config.h"
#include <Adafruit_GFX.h>
#include <string>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

Adafruit_ST7789 tft = Adafruit_ST7789(&SPI, TFT_CS, TFT_DC, TFT_RST);
int16_t currentCursorY = 0;
int16_t currentCursorX = 5;
bool screenCleared = false;

SemaphoreHandle_t bufferMutex = NULL;

int bufferHead = 0;
int bufferCount = 0;
int scrollOffset = 0;
std::string lineBuffer[SCROLL_BUFFER_SIZE];

void renderScreen(){
    if (bufferMutex != NULL) {
        xSemaphoreTake(bufferMutex, portMAX_DELAY);
    }
    
    Theme current = getCurrentTheme();
    tft.fillScreen(current.bg);
    tft.setTextColor(current.fg, current.bg);

    int total = bufferCount;
    int start = total - LINES_ON_SCREEN - scrollOffset;
    if (start < 0) start = 0;

    int y = 0;
    for(int i = start; i < total && i < start + LINES_ON_SCREEN; i++){
        int idx = (bufferHead - total + i + SCROLL_BUFFER_SIZE) % SCROLL_BUFFER_SIZE;
        tft.setCursor(5, y * LINE_HEIGHT);
        tft.println(lineBuffer[idx].c_str());
        y++;
    }

    currentCursorY = y * LINE_HEIGHT;

    if (scrollOffset > 0){
        tft.setTextColor(current.bg , current.bg ^ 0xFFFF);
        tft.setCursor(260, 0);
        char ind[16];
        snprintf(ind, sizeof(ind), "^%d", scrollOffset);
        tft.print(ind);
        vTaskDelay(2 / portTICK_PERIOD_MS);
        tft.endWrite();
        tft.setTextColor(current.fg, current.bg);
        
    }
    
    if (bufferMutex != NULL) {
        xSemaphoreGive(bufferMutex);
    }
}

void scrollUp(int lines){
    if (bufferMutex != NULL) {
        xSemaphoreTake(bufferMutex, portMAX_DELAY);
    }
    
    int oldOffset = scrollOffset;
    int maxScroll = bufferCount - LINES_ON_SCREEN;

    if (maxScroll < 0) maxScroll = 0;

    scrollOffset += lines;

    if (scrollOffset > maxScroll) scrollOffset = maxScroll;
    
    if (bufferMutex != NULL) {
        xSemaphoreGive(bufferMutex);
    }
    
    if (scrollOffset != oldOffset) {
        renderScreen();
    }
}

void scrollDown(int lines){
    if (bufferMutex != NULL) {
        xSemaphoreTake(bufferMutex, portMAX_DELAY);
    }
    
    int oldOffset = scrollOffset;
    scrollOffset -= lines;
    if (scrollOffset < 0) scrollOffset = 0;
    
    if (bufferMutex != NULL) {
        xSemaphoreGive(bufferMutex);
    }
    
    if (scrollOffset != oldOffset) {
        renderScreen();
    }
}

void scrollToBottom(){
    if (bufferMutex != NULL) {
        xSemaphoreTake(bufferMutex, portMAX_DELAY);
    }
    
    scrollOffset = 0;
    
    if (bufferMutex != NULL) {
        xSemaphoreGive(bufferMutex);
    }
    
    renderScreen();
}

void scrollToTop(){
    if (bufferMutex != NULL) {
        xSemaphoreTake(bufferMutex, portMAX_DELAY);
    }
    
    int maxScroll = bufferCount - LINES_ON_SCREEN;

    if (maxScroll < 0){
      maxScroll = 0;  
    } 

    scrollOffset = maxScroll;
    
    if (bufferMutex != NULL) {
        xSemaphoreGive(bufferMutex);
    }
    
    renderScreen();
}

uint8_t sin8(int angle) {
    return (uint8_t)((sin(angle * M_PI / 128.0) + 1.0) * 127.5);
}

void initDisplay() {
    delay(150);
    SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
    SPI.setFrequency(40000000); 
    tft.init(240, 320);
    tft.setRotation(1);
    tft.setTextWrap(true);
    tft.invertDisplay(false);
    for(int i = 0; i < SCROLL_BUFFER_SIZE; ++i){
        lineBuffer[i] = "\n";
    }
    
    if (bufferMutex == NULL) {
        bufferMutex = xSemaphoreCreateMutex();
    }
    
    applyTheme();
    currentCursorY = 0;
}


void applyTheme() {
    Theme current = getCurrentTheme();
    tft.fillScreen(current.bg);
    tft.setTextColor(current.fg);
    tft.setCursor(5, 0);
    currentCursorY = 0;
}

void clearScreen() {
    if (bufferMutex != NULL) {
        xSemaphoreTake(bufferMutex, portMAX_DELAY);
    }
    
    Theme current = getCurrentTheme();
    tft.fillScreen(current.bg);
    tft.setCursor(5, 0);
    currentCursorY = 0;
    scrollOffset = 0;
    tft.setTextColor(current.fg, current.bg);
    screenCleared = true;
    // print(">" + getDeviceName() + "@Mini:");
    
    if (bufferMutex != NULL) {
        xSemaphoreGive(bufferMutex);
    }
}

void newPage() {
    Theme current = getCurrentTheme();
    tft.fillScreen(current.bg);
    tft.setCursor(5, 0);
    currentCursorY = 0;
    tft.setTextColor(current.fg, current.bg);
}

void printLine(const std::string& s) {
    Serial.println(s.c_str());

    if (bufferMutex != NULL) {
        xSemaphoreTake(bufferMutex, portMAX_DELAY);
    }
    
    lineBuffer[bufferHead] = s;
    bufferHead = (bufferHead + 1) % SCROLL_BUFFER_SIZE;
    if (bufferCount < SCROLL_BUFFER_SIZE) bufferCount++;
    
    bool wasScrolled = (scrollOffset > 0);

    if (bufferMutex != NULL) {
        xSemaphoreGive(bufferMutex);
    }
    
    if (wasScrolled) {
        scrollToBottom();
        return;
    }
    
    if (scrollOffset == 0) {
        Theme current = getCurrentTheme();

        if (currentCursorY + LINE_HEIGHT > MAX_Y) {
            newPage();
            // renderScreen();

            vTaskDelay(1 / portTICK_PERIOD_MS);
            return;
        }

        tft.setCursor(5, currentCursorY);
        tft.setTextColor(current.fg, current.bg);
        tft.println(s.c_str());
        currentCursorY = tft.getCursorY();
        currentCursorX = tft.getCursorX();

        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

void printLineNoBuffer(const std::string& s) {
    Serial.println(s.c_str());

    if (bufferMutex != NULL) {
        xSemaphoreTake(bufferMutex, portMAX_DELAY);
    }
    
    bool wasScrolled = (scrollOffset > 0);
    
    if (bufferMutex != NULL) {
        xSemaphoreGive(bufferMutex);
    }
    
    if (wasScrolled) {
        scrollToBottom();
    }

    if (scrollOffset == 0) {
        Theme current = getCurrentTheme();

        if (currentCursorY + LINE_HEIGHT > MAX_Y) {
            newPage();
            // renderScreen();

            vTaskDelay(1 / portTICK_PERIOD_MS);
            return;
        }

        tft.setCursor(5, currentCursorY);
        tft.setTextColor(current.fg, current.bg);
        tft.println(s.c_str());
        currentCursorY = tft.getCursorY();
        currentCursorX = tft.getCursorX();

        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}


void print(const std::string& s) {
    Serial.print(s.c_str());

    if (bufferMutex != NULL) {
        xSemaphoreTake(bufferMutex, portMAX_DELAY);
    }
    
    Theme current = getCurrentTheme();
    if (currentCursorY > MAX_Y) {
        newPage();
    }
    tft.setTextColor(current.fg, current.bg);
    tft.print(s.c_str());
    currentCursorY = tft.getCursorY();
    currentCursorX = tft.getCursorX();
    
    if (bufferMutex != NULL) {
        xSemaphoreGive(bufferMutex);
    }
}

void print(const char& s) {
    Serial.print(s);

    if (bufferMutex != NULL) {
        xSemaphoreTake(bufferMutex, portMAX_DELAY);
    }
    
    Theme current = getCurrentTheme();
    if (currentCursorY > MAX_Y) {
        newPage();
    }
    tft.setTextColor(current.fg, current.bg);
    tft.print(s);
    currentCursorY = tft.getCursorY();
    currentCursorX = tft.getCursorX();
    
    if (bufferMutex != NULL) {
        xSemaphoreGive(bufferMutex);
    }
}




void addToBuffer(const std::string& s) {
    if (bufferMutex != NULL) {
        xSemaphoreTake(bufferMutex, portMAX_DELAY);
    }
    
    lineBuffer[bufferHead] = s;
    bufferHead = (bufferHead + 1) % SCROLL_BUFFER_SIZE;
    if (bufferCount < SCROLL_BUFFER_SIZE) bufferCount++;
    
    if (bufferMutex != NULL) {
        xSemaphoreGive(bufferMutex);
    }
}

void screensaver(int mode) {
    screenLocked = true;
    const int width  = 320;
    const int height = 230;

    tft.fillScreen(ST77XX_BLACK);
    Serial.println("Press ENTER to exit...");

    static uint16_t line[320];

    int offset = 0;
    const int targetFPS  = 20;
    const int frameDelay = 1000 / targetFPS;

    while (true) {
        unsigned long frameStart = millis();

        tft.startWrite();

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                uint16_t color = ST77XX_BLACK;

                switch (mode) {
                case 1: {
                    int wave = ((x + y + offset) % 40);
                    int brightness = (wave < 20) ? wave * 12 : (40 - wave) * 12;
                    color = tft.color565(0, brightness, brightness);
                } break;

                case 2: {
                    int pos = (x + offset * 2) % 192;
                    int r = 0, g = 0, b = 0;
                    if (pos < 64) { r = 255 - pos * 4; g = pos * 4; }
                    else if (pos < 128) { g = 255 - (pos-64) * 4; b = (pos-64) * 4; }
                    else { b = 255 - (pos-128) * 4; r = (pos-128) * 4; }
                    color = tft.color565(r, g, b);
                } break;

                case 3: {
                    int dx = abs(x - 160);
                    int dy = abs(y - 115);
                    int maxDist = (dx > dy) ? dx : dy;
                    int ring = (maxDist + offset) % 30;
                    int brightness = (ring < 15) ? ring * 17 : (30 - ring) * 17;
                    color = tft.color565(brightness, 0, 255 - brightness);
                } break;

                case 4: {
                    int gridX = (x + offset) >> 4;
                    int gridY = y >> 4;
                    int pulse = (gridX + gridY + (offset >> 2)) & 31;
                    int brightness = (pulse < 16) ? pulse * 16 : (32 - pulse) * 16;
                    color = tft.color565(brightness * 3, brightness >> 1, brightness << 1);
                } break;

                case 5: {
                    int cellSize = 20;
                    int cx = (x + offset) / cellSize;
                    int cy = y / cellSize;
                    if ((cx + cy) % 2 == 0) {
                        int brightness = ((x + offset) % cellSize) * 12;
                        color = tft.color565(brightness, brightness, 255);
                    }
                } break;

                case 6: {
                    int starSeed = (x * 17 + y * 13) % 1000;
                    if (starSeed < 5) {
                        int twinkle = (offset + starSeed * 7) % 30;
                        int brightness = twinkle < 15 ? twinkle * 17 : (30 - twinkle) * 17;
                        color = tft.color565(brightness, brightness, brightness);
                    }
                } break;

                case 7: {
                    int bar = ((x * offset) % 128);
                    int brightness = (bar < 64) ? bar * 6 : (256 - bar) * 6;
                    color = tft.color565(brightness, brightness >> 1, 0);
                } break;

                case 8: {
                    int cellSize = 10;
                    int cx = (x * offset) / cellSize;
                    int cy = (y + offset)/ cellSize;
                    if ((cx + cy) % 2 == 0) {
                        int brightness = ((x + offset) % cellSize) * 12;
                        color = tft.color565(brightness, brightness, 255);
                    } else if ((cx + cy) % 3 == 0) {
                        int brightness = ((x + offset) % cellSize) * 12;
                        color = tft.color565(brightness+20, brightness+20, 196);
                    }
                } break;

                default:
                    color = ((x + y + offset) % 60 < 30) ? ST77XX_CYAN : ST77XX_BLUE;
                    break;
                }

                line[x] = color;
            }

            tft.setAddrWindow(0, y, width, 1);
            tft.writePixels(line, width);

            if (y % 10 == 0) {
                tft.endWrite();
                vTaskDelay(1 / portTICK_PERIOD_MS);
                tft.startWrite();
            }
        }

        tft.endWrite();

        if (mode == 2)              offset += 4;
        else if (mode == 5 || mode == 7) offset += 3;
        else                        offset += 2;

        if (offset > 10000) offset = 0;

        tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
        tft.setTextSize(1);
        tft.setCursor(5, 230);
        tft.print("Press ENTER to exit...");

        if (Serial.available()) {
            char c = Serial.read();
            if (c == '\n') break;
        }

        unsigned long frameTime = millis() - frameStart;
        if (frameTime < frameDelay)
            vTaskDelay((frameDelay - frameTime) / portTICK_PERIOD_MS);
        else
            vTaskDelay(1 / portTICK_PERIOD_MS);
    }

    applyTheme();
    screenLocked = false;
    clearScreen();
}

void showLogo() {
    printLine("    __  ________   ______  ____  _____");
    printLine("   /  |/  /  _/ | / /  _/ / __ \\/ ___/");
    printLine("  / /|_/ // //  |/ // /  / / / /\\__ \\ ");
    printLine(" / /  / // // /|  // /  / /_/ /___/ / ");
    printLine("/_/  /_/___/_/ |_/___/  \\____//____/  ");
    printLine("");
    printLine(OS_VERSION);
    printLine("Check: https://github.com/VuqarAhadli");
}