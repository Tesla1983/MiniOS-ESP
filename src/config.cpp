#include "config.h"
#include <Arduino.h>


const char* NTP_SERVER = "pool.ntp.org";
const long GMT_OFFSET = 4 * 3600; /* you can change it depending on your timezone */
const int DAYLIGHT_OFFSET = 0;
const char* OS_VERSION = "MiniOS-ESP v2.0.4";

std::string deviceName = "Mini";

std::string getDeviceName() {
    return deviceName;
}

void setDeviceName(const std::string& name) {
    if (!name.empty() && name.length() <= 32) {
        deviceName = name;
    }
}
