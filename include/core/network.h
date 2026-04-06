#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <string>
#include "userspace/network_utils.h"


extern std::string WIFI_SSID;
extern std::string WIFI_PASS;


enum NetworkStatus {
    NET_DISCONNECTED,
    NET_CONNECTING,
    NET_CONNECTED,
    NET_FAILED
};

extern NetworkStatus networkStatus;

void connectWiFi(bool useConfig);
void disconnectWiFi();
void scanWiFi();
void showNetworkInfo();
bool isConnected();
std::string getLocalIP();
int getSignalStrength();

#endif