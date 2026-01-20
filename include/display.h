#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Adafruit_ST7789.h>
#include <string>

extern Adafruit_ST7789 tft;
extern bool screenLocked;
extern bool screenCleared;
extern int16_t currentCursorY;

#define MAX_Y 230

void initDisplay();
void applyTheme();
void clearScreen();
void printLine(const std::string& s);
void print(const std::string& s);
void showLogo();
void screensaver(int mode);
#endif