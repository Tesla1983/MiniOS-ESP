#ifndef THEME_H
#define THEME_H

#include <Arduino.h>
#include <string>

struct Theme {
    uint16_t bg;
    uint16_t fg;
    const char* name;
};

extern Theme themes[];
extern int currentTheme;
extern int themeCount;

void listThemes();
void setTheme(const std::string& themeName);
Theme getCurrentTheme();

#endif