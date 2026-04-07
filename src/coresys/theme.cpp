#include "theme.h"
#include "config.h"
#include "display.h"
#include "kernel.h"
#include <Adafruit_ST7789.h>
#include <string>

Theme themes[] = {
    {ST77XX_BLACK, ST77XX_WHITE, "classic"},
    {ST77XX_BLACK, ST77XX_GREEN, "matrix"},
    {ST77XX_BLACK, ST77XX_CYAN, "cyan"},
    {ST77XX_BLUE, ST77XX_YELLOW, "blue"},
    {ST77XX_WHITE, ST77XX_BLACK, "light"},
    {ST77XX_BLACK, ST77XX_ORANGE, "dark-orange"}, 
    {0x2104, ST77XX_MAGENTA, "purple"},     
    {ST77XX_BLACK, ST77XX_RED, "red-night"},
    {ST77XX_BLACK, 0xFFE0, "sunshine"},    
    {ST77XX_BLUE, 0x07FF, "skyline"},       
    {0xFD20, ST77XX_BLACK, "orange-flame"}, 
    {ST77XX_WHITE, 0xF81F, "cotton-candy"},
    {0x8410, ST77XX_WHITE, "pastel-gray"},  
    {0x03E0, ST77XX_WHITE, "lime-light"},   
    {ST77XX_RED, 0xFFE0, "fireworks"},      
    {0x001F, 0xF81F, "neon-night"},         
    {0x7E0, 0xF81F, "tropical"},            
    {0xFC00, 0x07FF, "sunset"},             
};

int currentTheme = 0;
int themeCount = sizeof(themes) / sizeof(Theme);



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
            saveSavedTheme(currentTheme);
            printLine("[SYSTEM] Theme set: " + std::string(themes[currentTheme].name));
            logKernelMessage("[SYSTEM] Theme set: " + std::string(themes[currentTheme].name));
            return;
        }
    }
    
    try {
        int themeNum = std::stoi(tn);
        if (themeNum >= 0 && themeNum < themeCount) {
            currentTheme = themeNum;
            applyTheme();
            saveSavedTheme(currentTheme);
            printLine("[SYSTEM] Theme set: " + std::string(themes[currentTheme].name));
            logKernelMessage("[SYSTEM] Theme set: " + std::string(themes[currentTheme].name));
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