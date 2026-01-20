#include "theme.h"
#include "display.h"
#include <Adafruit_ST7789.h>
#include <string>

Theme themes[] = {
    {ST77XX_BLACK, ST77XX_GREEN, "matrix"},
    {ST77XX_BLACK, ST77XX_CYAN, "cyan"},
    {ST77XX_BLACK, ST77XX_WHITE, "classic"},
    {ST77XX_BLUE, ST77XX_YELLOW, "blue"},
    {ST77XX_WHITE, ST77XX_BLACK, "light"},
    {0x0014, ST77XX_ORANGE, "dark-orange"},
    {0x2104, ST77XX_MAGENTA, "purple"},
    {ST77XX_BLACK, ST77XX_RED, "red-night"}
};

int currentTheme = 0;
int themeCount = sizeof(themes) / sizeof(Theme);

void initThemes() {}

void listThemes() {
    printLine("Available themes:");
    for (int i = 0; i < themeCount; i++) {
        std::string marker = (i == currentTheme) ? " *" : "";
        printLine(std::to_string(i) + ": " + themes[i].name + marker);
    }
}

void setTheme(const std::string& tn) {
    for (int i = 0; i < themeCount; i++) {
        if (tn == themes[i].name) {
            currentTheme = i;
            applyTheme();
            printLine("Theme set: " + std::string(themes[currentTheme].name));
            return;
        }
    }
    
    try {
        int themeNum = std::stoi(tn);
        if (themeNum >= 0 && themeNum < themeCount) {
            currentTheme = themeNum;
            applyTheme();
            printLine("Theme set: " + std::string(themes[currentTheme].name));
            return;
        }
    } catch (...) {
    }
    
    printLine("Invalid theme.");
    printLine("Use 'themes' to list.");
}

Theme getCurrentTheme() {
    return themes[currentTheme];
}