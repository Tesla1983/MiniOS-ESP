#include "mirror.h"
#include "display.h"
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <TJpg_Decoder.h>

#define SERVER_HOST  "192.168.1.119" /// hardcoded local IP (will be cahnged)
#define SERVER_PORT  8000 /// flask python port
#define SERVER_PATH  "/frame.jpg"
#define FETCH_INTERVAL_MS  100
#define HTTP_TIMEOUT_MS  2000
#define JPEG_BUF_MAX  32768
#define MIRROR_W 320
#define MIRROR_H 240

static WiFiClient optimisedClient;
static uint8_t* jpegBuf = nullptr;

static IRAM_ATTR bool jpeg_to_tft(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
    if (x >= MIRROR_W || y >= MIRROR_H) return true;
    uint16_t cols = (x + w > MIRROR_W) ? (MIRROR_W - x) : w;
    tft.startWrite();
    for (uint16_t row = 0; row < h && (y + row) < MIRROR_H; row++) {
        tft.setAddrWindow(x, y + row, cols, 1);
        tft.writePixels(&bitmap[row * w], cols);
    }
    tft.endWrite();
    return true;
}

static bool fetchAndDraw() {
    if (!optimisedClient.connected()) {
        optimisedClient.stop();
        if (!optimisedClient.connect(SERVER_HOST, SERVER_PORT)) {
            Serial.printf("[mirror] connection failed ADDRESS: %s:%i\n",SERVER_HOST,SERVER_PORT);
            return false;
        }
    }

    /*
        GET /frame.jpg HTTP/1.1
        Host: 192.168.1.xxx 
        Connection: keep-alive
    */
    optimisedClient.print(
        "GET " SERVER_PATH " HTTP/1.1\r\n"
        "Host: " SERVER_HOST "\r\n"
        "Connection: keep-alive\r\n" 
        "Accept: image/jpeg\r\n"
        "\r\n"
    );

    uint32_t t = millis();
    int contentLength = -1;
    bool jobDone = false;
    while (!jobDone && (millis() - t) < HTTP_TIMEOUT_MS) {
        if (!optimisedClient.connected()) return false;
        char line[128];
        if (optimisedClient.available()) {
            
            size_t idx = 0;
            while (!jobDone && (millis() - t) < HTTP_TIMEOUT_MS) {
                if (optimisedClient.available()) {
                    char c = optimisedClient.read();

                    if (c == '\n') {
                        line[idx] = 0;

                        if (idx == 1 && line[0] == '\r')
                            jobDone = true;

                        if (strncmp(line, "Content-Length:", 15) == 0)
                            contentLength = atoi(line + 15);

                        idx = 0;
                    } else if (idx < sizeof(line) - 1) {
                        line[idx++] = c;
                    }
                }else{
                    vTaskDelay(pdMS_TO_TICKS(1));
                }
                
            }
        }
    }
    if (!jobDone || contentLength <= 0 || contentLength > JPEG_BUF_MAX) {
        Serial.printf("[mirror] bad headers! Content length=%d\n", contentLength);
        optimisedClient.stop();
        return false;
    }

    jpegBuf = (uint8_t*)(psramFound() ? ps_malloc(JPEG_BUF_MAX) : malloc(JPEG_BUF_MAX));
    if (!jpegBuf) { Serial.println("[mirror] alloc failed"); optimisedClient.stop(); return false; }
    size_t total = 0;
    t = millis();
    while (total < (size_t)contentLength && (millis() - t) < HTTP_TIMEOUT_MS) {
        if (!optimisedClient.connected()) break;
        int available = optimisedClient.available();
        if (available > 0)
            total += optimisedClient.readBytes((char*)jpegBuf + total, min((size_t)available, (size_t)contentLength - total));
        else
            delayMicroseconds(200);
    }

    bool ok = false;
    if (total == (size_t)contentLength) {
        TJpgDec.drawJpg(0, 0, jpegBuf, total);
        ok = true;
    } else {
        Serial.printf("[mirror] insufficient header read %d/%d\n", total, contentLength);
        optimisedClient.stop();
    }
    free(jpegBuf);
    jpegBuf = nullptr;
    return ok;
}

void startMirror() {
    screenLocked = true;
    TJpgDec.setJpgScale(1);
    TJpgDec.setSwapBytes(false);
    TJpgDec.setCallback(jpeg_to_tft);
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.setTextSize(1);
    tft.setCursor(5, 230);
    print("Mirroring... ENTER to exit");


    uint32_t lastFetched = 0;
    while (true) {
        if (Serial.available()) {
            if (Serial.read() == '\n') break;
        }
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("[mirror] wifi lost...");
            optimisedClient.stop();
            vTaskDelay(pdMS_TO_TICKS(450));
            continue;
        }
        if (millis() - lastFetched >= FETCH_INTERVAL_MS) {
            lastFetched = millis();
            uint32_t t0 = millis();

            if (fetchAndDraw()) {
                uint32_t t1 = millis();
                uint32_t f = t1 - t0;
                if (f > 0)
                Serial.printf("[mirror] Fetched: %lu ms FPS: %lu\n", f, 1000 / f);
                else
                Serial.println("[mirror] Fetched: <1ms");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    optimisedClient.stop();
    applyTheme();
    screenLocked = false;
    clearScreen();
}