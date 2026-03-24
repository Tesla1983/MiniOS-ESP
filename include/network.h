#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <string>


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

void curlURL(const std::string& url);
void curlURLVerbose(const std::string& url);
std::string httpGet(const std::string& url);
int httpPost(const std::string& url, const std::string& data);

void pingHost(const std::string& host);
void dnsLookup(const std::string& hostname);

std::string getStatusText(int httpCode);
std::string formatBytes(int bytes);
bool isBinaryContent(const std::string& contentType);

struct CurlOptions {
    std::string url;
    std::string method = "GET";
    std::string data = "";
    bool verbose = false;
    bool followRedirects = true;
    int timeout = 15000;
    std::string userAgent = "MiniOS-curl/1.0";

    std::string headers[5];
    int headerCount = 0;
    
    void addHeader(const std::string& header) {
        if (headerCount < 5) {
            headers[headerCount++] = header;
        }
    }
};

void curlWithOptions(CurlOptions opts);

struct CurlParams {
    std::string url;
    bool verbose;
};

void curlProcess(void* parameter);

#endif