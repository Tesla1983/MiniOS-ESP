#ifndef THEME_H
#define THEME_H

#include <Arduino.h>
#include <string>

struct Prompt {
    uint16_t name;
    uint16_t at;
    uint16_t host;
    uint16_t aux;
};

struct Theme {
    uint16_t bg;
    uint16_t fg;
    Prompt prompt;
    const char* name;
};

extern Theme themes[];
extern int currentTheme;
extern int themeCount;
extern Theme current;
void listThemes();
void setTheme(const std::string& themeName);
Theme getCurrentTheme();

#endif