#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

#include <string>

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

void curlURL(const std::string& url);
void curlURLVerbose(const std::string& url);
void curlWithOptions(CurlOptions opts);
std::string httpGet(const std::string& url);
int httpPost(const std::string& url, const std::string& data);

void pingHost(const std::string& host);
void dnsLookup(const std::string& hostname);

std::string getStatusText(int httpCode);
std::string formatBytes(int bytes);
bool isBinaryContent(const std::string& contentType);

#endif