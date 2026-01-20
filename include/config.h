#ifndef CONFIG_H
#define CONFIG_H

#include <string>

#define TFT_CS   5
#define TFT_DC   2  // Display Pins for ST7789 review README.me for full wiring setup
#define TFT_RST  4


extern const char* NTP_SERVER;
extern const long GMT_OFFSET;
extern const int DAYLIGHT_OFFSET;
extern const char* OS_VERSION;
extern std::string deviceName;

std::string getDeviceName();
void setDeviceName(const std::string& name);

#endif
