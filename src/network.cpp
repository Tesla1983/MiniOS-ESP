#include "network.h"
#include "display.h"
#include "timeutils.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Ping.h>
#include <string>

std::string WIFI_SSID = "";
std::string WIFI_PASS = "";
NetworkStatus networkStatus = NET_DISCONNECTED;
extern bool inputLocked;

void connectWiFi() {
    inputLocked = true;
    vTaskDelay(100 / portTICK_PERIOD_MS);
    
    if (WiFi.status() == WL_CONNECTED) {
        printLine("Already connected!");
        printLine("SSID: " + std::string(WiFi.SSID().c_str()));
        printLine("IP: " + std::string(WiFi.localIP().toString().c_str()));
        printLine("RSSI: " + std::to_string(WiFi.RSSI()) + " dBm");
        inputLocked = false;
        return;
    }
    
    while (Serial.available() > 0) Serial.read();
    
    printLine("Enter SSID: ");
    WIFI_SSID = "";
    while (true) {
        if (Serial.available()) {
            char c = Serial.read();
            if (c == '\n' || c == '\r') {
                if (WIFI_SSID.length() > 0) {
                    Serial.println();
                    break;
                }
            } else if (c == '\b' || c == 127) {
                if (WIFI_SSID.length() > 0) {
                    WIFI_SSID.pop_back();
                    Serial.write('\b'); Serial.write(' '); Serial.write('\b');
                }
            } else if (c >= 32 && c <= 126) {
                WIFI_SSID += c;
                Serial.write(c);
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    WIFI_SSID.erase(0, WIFI_SSID.find_first_not_of(" \t\n\r\f\v"));
    WIFI_SSID.erase(WIFI_SSID.find_last_not_of(" \t\n\r\f\v") + 1);
    
    while (Serial.available() > 0) Serial.read();
    
    printLine("Enter Password: ");
    WIFI_PASS = "";
    while (true) {
        if (Serial.available()) {
            char c = Serial.read();
            if (c == '\n' || c == '\r') {
                if (WIFI_PASS.length() > 0) {
                    Serial.println();
                    break;
                }
            } else if (c == '\b' || c == 127) {
                if (WIFI_PASS.length() > 0) {
                    WIFI_PASS.pop_back();
                    Serial.write('\b'); Serial.write(' '); Serial.write('\b');
                }
            } else if (c >= 32 && c <= 126) {
                WIFI_PASS += c;
                Serial.write('*');
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    WIFI_PASS.erase(0, WIFI_PASS.find_first_not_of(" \t\n\r\f\v"));
    WIFI_PASS.erase(WIFI_PASS.find_last_not_of(" \t\n\r\f\v") + 1);
    
    inputLocked = false;
    
    printLine("Connecting to: " + WIFI_SSID);
    WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        Serial.print(".");
        tft.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        printLine("");
        printLine("Connected!");
        printLine("SSID: " + std::string(WiFi.SSID().c_str()));
        printLine("IP: " + std::string(WiFi.localIP().toString().c_str()));
        printLine("RSSI: " + std::to_string(WiFi.RSSI()) + " dBm");
        syncTime();
    } else {
        printLine("");
        printLine("Failed to connect.");
    }
}

void disconnectWiFi() {
    if (WiFi.status() != WL_CONNECTED) {
        printLine("Not connected to WiFi");
        return;
    }
    
    WiFi.disconnect();
    networkStatus = NET_DISCONNECTED;
    printLine("WiFi disconnected");
}

void scanWiFi() {
    printLine("Scanning networks...");
    
    int n = WiFi.scanNetworks();
    
    if (n == 0) {
        printLine("No networks found");
    } else {
        printLine("Found " + std::to_string(n) + " networks:");
        printLine("");
        printLine("  SSID                   RSSI  Ch  Enc");
        printLine("  ----------------------------------------");
        
        for (int i = 0; i < n; i++) {
            String ssid_s = WiFi.SSID(i);
            std::string ssid = std::string(ssid_s.c_str());
            if (ssid.length() > 20) ssid = ssid.substr(0, 20);
            
            std::string encryption;
            switch (WiFi.encryptionType(i)) {
                case WIFI_AUTH_OPEN: encryption = "Open"; break;
                case WIFI_AUTH_WEP: encryption = "WEP"; break;
                case WIFI_AUTH_WPA_PSK: encryption = "WPA"; break;
                case WIFI_AUTH_WPA2_PSK: encryption = "WPA2"; break;
                case WIFI_AUTH_WPA_WPA2_PSK: encryption = "WPA/2"; break;
                default: encryption = "WPA2"; break;
            }
            
            char line[60];
            sprintf(line, "%2d %-20s %4d  %2d  %s",
                    i + 1,
                    ssid.c_str(),
                    WiFi.RSSI(i),
                    WiFi.channel(i),
                    encryption.c_str());
            printLine(line);
        }
    }
    
    WiFi.scanDelete();
}

void showNetworkInfo() {
    if (WiFi.status() != WL_CONNECTED) {
        printLine("Not connected to WiFi");
        return;
    }
    
    printLine("IP Address: " + std::string(WiFi.localIP().toString().c_str()));
    printLine("Gateway: " + std::string(WiFi.gatewayIP().toString().c_str()));
    printLine("Subnet: " + std::string(WiFi.subnetMask().toString().c_str()));
    printLine("DNS: " + std::string(WiFi.dnsIP().toString().c_str()));
    printLine("MAC: " + std::string(WiFi.macAddress().c_str()));
    printLine("RSSI: " + std::to_string(WiFi.RSSI()) + " dBm");
    printLine("Channel: " + std::to_string(WiFi.channel()));
}

bool isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

std::string getLocalIP() {
    return std::string(WiFi.localIP().toString().c_str());
}

int getSignalStrength() {
    return WiFi.RSSI();
}


void curlURL(const std::string& url) {
    CurlOptions opts;
    opts.url = url;
    opts.verbose = false;
    curlWithOptions(opts);
}

void curlURLVerbose(const std::string& url) {
    CurlOptions opts;
    opts.url = url;
    opts.verbose = true;
    curlWithOptions(opts);
}

void curlWithOptions(CurlOptions opts) {
    if (!isConnected()) {
        printLine("curl: not connected to WiFi");
        printLine("Run 'wifi' first");
        return;
    }

    if (opts.url.substr(0, 7) != "http://" && opts.url.substr(0, 8) != "https://") {
        printLine("curl: invalid URL (must start with http:// or https://)");
        return;
    }

    HTTPClient http;
    http.begin(opts.url.c_str());
    http.setTimeout(opts.timeout);
    
    if (opts.followRedirects) {
        http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    }
    
    http.setUserAgent(opts.userAgent.c_str());
    
    for (int i = 0; i < opts.headerCount; i++) {
        int colonPos = opts.headers[i].find(':');
        if (colonPos > 0) {
            std::string headerName = opts.headers[i].substr(0, colonPos);
            std::string headerValue = opts.headers[i].substr(colonPos + 1);
            headerValue.erase(0, headerValue.find_first_not_of(" \t"));
            headerValue.erase(headerValue.find_last_not_of(" \t") + 1);
            http.addHeader(headerName.c_str(), headerValue.c_str());
        }
    }
    
    if (opts.verbose) {
        printLine("> " + opts.method + " " + opts.url);
        printLine("> User-Agent: " + opts.userAgent);
    }
    
    unsigned long startTime = millis();
    int code;
    
    if (opts.method == "POST") {
        code = http.POST(opts.data.c_str());
    } else if (opts.method == "PUT") {
        code = http.PUT(opts.data.c_str());
    } else if (opts.method == "DELETE") {
        code = http.sendRequest("DELETE");
    } else {
        code = http.GET();
    }
    
    unsigned long duration = millis() - startTime;
    
    if (code <= 0) {
        printLine("curl: connection failed");
        printLine("Error: " + std::string(http.errorToString(code).c_str()));
        http.end();
        return;
    }

    if (opts.verbose) {
        printLine("< HTTP/1.1 " + std::to_string(code) + " " + getStatusText(code));
        printLine("< Request-Time: " + std::to_string(duration) + "ms");
    } else {
        printLine("HTTP " + std::to_string(code) + " - " + std::to_string(duration) + "ms");
    }
    
    if (opts.verbose) {
        if (http.hasHeader("Content-Type")) {
            printLine("< Content-Type: " + std::string(http.header("Content-Type").c_str()));
        }
        if (http.hasHeader("Content-Length")) {
            printLine("< Content-Length: " + std::string(http.header("Content-Length").c_str()));
        }
        if (http.hasHeader("Server")) {
            printLine("< Server: " + std::string(http.header("Server").c_str()));
        }
        printLine("");
    }
    
    if (code == HTTP_CODE_OK) {
        int contentLength = http.getSize();
        String contentType_s = http.header("Content-Type");
        std::string contentType = std::string(contentType_s.c_str());
        
        if (isBinaryContent(contentType)) {
            printLine("Binary content (" + contentType + ")");
            printLine("Size: " + formatBytes(contentLength));
            printLine("Cannot display binary data");
        } else {
           
            String payload_s = http.getString();
            std::string payload = std::string(payload_s.c_str());
            int len = payload.length();
            
            if (len > 1500) {
                printLine(payload.substr(0, 1500));
                printLine("");
                printLine("... (+" + std::to_string(len - 1500) + " bytes)");
                printLine("Response truncated at 1500 bytes");
            } else if (len > 0) {
                printLine(payload);
            } else {
                printLine("(empty response)");
            }
        }
    } else if (code >= 300 && code < 400) {
        if (http.hasHeader("Location")) {
            printLine("Redirect to: " + std::string(http.header("Location").c_str()));
        }
    } else if (code >= 400) {
        
        printLine("Error " + std::to_string(code) + ": " + getStatusText(code));
        String payload_s = http.getString();
        std::string payload = std::string(payload_s.c_str());
        if (payload.length() > 300) {
            printLine(payload.substr(0, 300) + "...");
        } else if (payload.length() > 0) {
            printLine(payload);
        }
    }

    http.end();
}

std::string httpGet(const std::string& url) {
    if (!isConnected()) return "";
    
    HTTPClient http;
    http.begin(url.c_str());
    http.setTimeout(10000);
    
    int code = http.GET();
    std::string result = "";
    
    if (code == HTTP_CODE_OK) {
        result = std::string(http.getString().c_str());
    }
    
    http.end();
    return result;
}

int httpPost(const std::string& url, const std::string& data) {
    if (!isConnected()) return -1;
    
    HTTPClient http;
    http.begin(url.c_str());
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(10000);
    
    int code = http.POST(data.c_str());
    http.end();
    
    return code;
}


void pingHost(const std::string& host) {
    if (!isConnected()) {
        printLine("ping: not connected to WiFi");
        return;
    }
    
    printLine("PING " + host);
    
    IPAddress ip;
    if (!WiFi.hostByName(host.c_str(), ip)) {
        printLine("ping: cannot resolve " + host);
        return;
    }
    
    printLine("Pinging " + std::string(ip.toString().c_str()) + "...");
    
    int sent = 0;
    int received = 0;
    float totalTime = 0;
    float minTime = 999999;
    float maxTime = 0;
    
    for (int i = 0; i < 4; i++) {
        sent++;
        
        bool success = Ping.ping(ip, 1);
        
        if (success) {
            received++;
            float time = Ping.averageTime();
            totalTime += time;
            
            if (time < minTime) minTime = time;
            if (time > maxTime) maxTime = time;
            
            printLine(std::to_string(i + 1) + ": Reply from " + std::string(ip.toString().c_str()) + 
                     " time=" + std::to_string(time) + "ms");
        } else {
            printLine(std::to_string(i + 1) + ": Request timeout");
        }
        
        if (i < 3) {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
    
    printLine("");
    printLine("--- " + host + " ping statistics ---");
    printLine(std::to_string(sent) + " packets sent, " + std::to_string(received) + " received, " +
             std::to_string(((sent - received) * 100) / sent) + "% packet loss");
    
    if (received > 0) {
        float avgTime = totalTime / received;
        printLine("Round-trip min/avg/max = " + 
                 std::to_string(minTime) + "/" + 
                 std::to_string(avgTime) + "/" + 
                 std::to_string(maxTime) + " ms");
    }
}

void dnsLookup(const std::string& hostname) {
    if (!isConnected()) {
        printLine("dns: not connected to WiFi");
        return;
    }
    
    printLine("Looking up: " + hostname);
    
    IPAddress ip;
    if (WiFi.hostByName(hostname.c_str(), ip)) {
        printLine("IP Address: " + std::string(ip.toString().c_str()));
    } else {
        printLine("DNS lookup failed");
    }
}


std::string getStatusText(int code) {
    switch(code) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 304: return "Not Modified";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 429: return "Too Many Requests";
        case 500: return "Internal Server Error";
        case 502: return "Bad Gateway";
        case 503: return "Service Unavailable";
        case 504: return "Gateway Timeout";
        default: return "";
    }
}

std::string formatBytes(int bytes) {
    if (bytes < 1024) {
        return std::to_string(bytes) + " B";
    } else if (bytes < 1024 * 1024) {
        char buffer[16];
        sprintf(buffer, "%.2f KB", bytes / 1024.0);
        return std::string(buffer);
    } else {
        char buffer[16];
        sprintf(buffer, "%.2f MB", bytes / 1024.0 / 1024.0);
        return std::string(buffer);
    }
}

bool isBinaryContent(const std::string& contentType) {
    std::string lowercase = contentType;
    std::transform(lowercase.begin(), lowercase.end(), lowercase.begin(), ::tolower);
    return lowercase.find("image/") != std::string::npos ||
           lowercase.find("video/") != std::string::npos ||
           lowercase.find("audio/") != std::string::npos ||
           lowercase.find("application/pdf") != std::string::npos ||
           lowercase.find("application/zip") != std::string::npos ||
           lowercase.find("application/octet-stream") != std::string::npos;
}



void curlProcess(void* parameter) {
    CurlParams* params = (CurlParams*)parameter;
    
    extern bool inputLocked;
    inputLocked = true;
    
    if (params->verbose) {
        curlURLVerbose(params->url);
    } else {
        curlURL(params->url);
    }
    
    delete params;
    inputLocked = false;
    
    vTaskDelete(NULL);
}