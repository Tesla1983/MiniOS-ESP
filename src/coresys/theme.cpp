#include "theme.h"
#include "config.h"
#include "display.h"
#include "kernel.h"
#include <Adafruit_ST7789.h>
#include <string>
    // uint16_t name;
    // uint16_t at;
    // uint16_t host;
    // uint16_t aux;
Theme themes[] = {
    {ST77XX_BLACK, ST77XX_WHITE, {0xdfc9,0x0693,0x071d,0x0ec8}, "classic"},

    {ST77XX_BLACK, ST77XX_GREEN, {0xfca0,0xde69,0x231c,0xb27c},  "matrix"},

    {ST77XX_BLACK, ST77XX_CYAN, {0xb61c,0xafa5,0xe63b,0x569d},  "cyan"},

    {ST77XX_BLUE, ST77XX_YELLOW, {0x07ff,0xf800,0xfec0,0xffff},  "blue"},

    {ST77XX_WHITE, ST77XX_BLACK, {0x9a93,0xa4a9,0x33e6,0x9bd2},  "light"},

    {ST77XX_BLACK, ST77XX_ORANGE, {0xf878,0xad5a,0x071d,0x0ec8},  "dark-orange"}, 

    {0x2104, ST77XX_MAGENTA, {0xfed0,0xedb4,0x9ed0,0xfd5a},  "purple"},     

    {ST77XX_BLACK, ST77XX_RED, {0x1fe0,0xeffd,0x1e95,0xdeff},  "red-night"},

    {ST77XX_BLACK, 0xFFE0, {0x099f,0x0693,0x90de,0xeb46},  "sunshine"},    

    // {ST77XX_BLUE, 0x07FF, "skyline"},       
    // {0xFD20, ST77XX_BLACK, "orange-flame"}, 
    // {ST77XX_WHITE, 0xF81F, "cotton-candy"},
    // {0x8410, ST77XX_WHITE, "pastel-gray"},  
    // {0x03E0, ST77XX_WHITE, "lime-light"},   
    // {ST77XX_RED, 0xFFE0, "fireworks"},      
    // {0x001F, 0xF81F, "neon-night"},         
    // {0x7E0, 0xF81F, "tropical"},            
    // {0xFC00, 0x07FF, "sunset"},             
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