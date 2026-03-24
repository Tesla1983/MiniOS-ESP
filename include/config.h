#ifndef CONFIG_H
#define CONFIG_H
#include <string>

// PINS
#define TFT_CS   5
#define TFT_DC   2
#define TFT_RST  4
// #define MAX_Y    230

// TIME
extern const char* NTP_SERVER;
extern const long  GMT_OFFSET;
extern const int   DAYLIGHT_OFFSET;

// OS INFO
extern const char* OS_VERSION;

// USERSPACE PERSISTENCE
void loadConfig();      
void saveConfig();      

std::string getDeviceName();
void setDeviceName(const std::string& name);

int  getSavedTheme();          
void saveSavedTheme(int index);

void setWifiConfig(const std::string& SSID, const std::string& PASS);
std::string getWifiSSID();
std::string getWifiPass();

#endif