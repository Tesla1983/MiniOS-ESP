#include "display.h"
#include "d20.h"
#include <cstdlib>
#include <time.h>

static const int CX = X_MAX / 2;
static const int CY = Y_MAX / 2 - 10;
static const int R  = 80;  

static void drawD20(int val, uint16_t faceColor, uint16_t borderColor) {
    int px[6], py[6];
    for (int i = 0; i < 6; i++) {
        float angle = -M_PI / 2 + i * 2 * M_PI / 6;
        px[i] = CX + R * cos(angle);
        py[i] = CY + R * sin(angle);
    }

    for (int i = 0; i < 6; i++) {
        int ni = (i + 1) % 6;
        tft.fillTriangle(CX, CY, px[i], py[i], px[ni], py[ni], faceColor);
    }

    for (int i = 0; i < 6; i++) {
        tft.drawLine(CX, CY, px[i], py[i], borderColor);
    }
    for (int i = 0; i < 6; i++) {
        int ni = (i + 1) % 6;
        tft.drawLine(px[i], py[i], px[ni], py[ni], borderColor);
    }

    tft.setTextSize(3);
    tft.setTextColor(borderColor, faceColor);
    int digits = (val >= 10) ? 2 : 1;
    tft.setCursor(CX - digits * 9, CY - 12);
    tft.print(val);
    tft.setTextSize(1);
}

static void eraseD20() {
    tft.fillRect(CX - R - 4, CY - R - 4, (R + 4) * 2, (R + 4) * 2, ST77XX_BLACK);
}

void d20Game() {
    std::srand(time(nullptr));
    tft.fillScreen(ST77XX_BLACK);
    screenLocked = true;

    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.setTextSize(1);
    tft.setCursor(5, 220);
    tft.print("ENTER=exit  SPACE=roll");

    tft.setCursor(CX - 40, CY - 10);
    tft.setTextSize(1);
    tft.print("Press SPACE");
    tft.setCursor(CX - 40 , CY + 2);
    tft.print("to roll...");

    int lastVal = -1;
    int rolls   = 0;
    int total   = 0;

    while (true) {
        if (Serial.available()) {
            char c = Serial.read();

            if (c == '\n') {
                clearScreen();
                screenLocked = false;
                return;
            }

            bool doRoll = (c == ' ');

            if (c == '\x1b' && Serial.available() >= 2) {
                Serial.read(); Serial.read();  
                doRoll = true;
            }

            if (doRoll) {
                int val = (std::rand() % 20) + 1;

                for (int f = 0; f < 4; f++) {
                    eraseD20();
                    int fake = (std::rand() % 20) + 1;
                    drawD20(fake, ST77XX_BLUE, ST77XX_WHITE);
                    vTaskDelay(60 / portTICK_PERIOD_MS);
                }

                eraseD20();

                uint16_t face   = ST77XX_BLUE;
                uint16_t border = ST77XX_WHITE;
                if (val == 20) {
                    face = 0x07E0;
                    border = ST77XX_WHITE;
                }  
                if (val == 1) { 
                    face = ST77XX_RED;
                    border = ST77XX_YELLOW;
                }

                drawD20(val, face, border);
                tft.fillRect(5,180,80,8,ST77XX_BLACK);
                tft.setTextSize(1);
                tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
                tft.setCursor(5, 180);
                if (val == 20){
                    tft.setTextColor(ST77XX_GREEN);
                    tft.print("20 | Lucky!");
                    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
                }
                else if (val == 1){
                    tft.setTextColor(ST77XX_RED);
                    tft.print("1  | Unlucky :(");
                    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
                }  
                else {
                    tft.print("Rolled: ");
                    tft.print(val);
                }

                rolls++;
                total += val;
                tft.fillRect(5,200,200,8,ST77XX_BLACK);
                tft.setCursor(5, 200);
                tft.print("Rolls: ");
                tft.print(rolls);
                tft.print("  Avg: ");
                tft.print((float)total / rolls, 1);

                lastVal = val; 
                Serial.print("Rolled: "); Serial.println(val);
            }
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}