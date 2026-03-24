#ifndef DISPLAY_H
#define DISPLAY_H
#include <Arduino.h>
#include <Adafruit_ST7789.h>
#include <string>

extern Adafruit_ST7789 tft;
extern bool screenLocked;
extern bool screenCleared;
extern int16_t currentCursorY;

#define MAX_Y 240

void initDisplay();
void applyTheme();
void clearScreen();
void newPage();
void renderScreen();
void printLine(const std::string& s);
void print(const std::string& s);

void scrollUp(int lines = 3);
void scrollDown(int lines = 3);
void scrollToBottom();
void scrollToTop();
void screensaver(int mode);
void showLogo();
uint8_t sin8(int angle);

#endif