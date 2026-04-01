#include "display.h"
#include "pingpong.h"
#include <cstdlib>
#include <time.h>
#include <algorithm> 

static void fillBorders(){
    tft.fillRect(0,0,315,5,ST77XX_BLUE);
    tft.fillRect(0,224,315,5,ST77XX_BLUE);
    tft.fillRect(315,0,5,229,ST77XX_RED);
}

static int optimisedBorderRedraw =0;

void pingpongGame() {
    std::srand(time(nullptr));

    tft.fillScreen(ST77XX_BLACK);
    screenLocked = true;
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.setTextSize(1);
    tft.setCursor(5, 230);
    tft.print("Press ENTER to exit...");
    Serial.println("Press ENTER to exit...");

    int x = X_MAX / 2;
    int y = Y_MAX / 2;
    int speedX = 1;
    int speedY = 2;
    int ballRadius = 5;
    uint16_t colour = 65535;

    uint16_t racketX = 0;
    uint16_t racketY = Y_MAX / 2;
    tft.drawRect(racketX, racketY, RACKET_WIDTH, RACKET_LENGTH, colour);

    while (true) {
        if (Serial.available()) {
            char c = Serial.read();

            if (c == '\n') { 
                clearScreen();
                screenLocked = false;
                return;
            }

            if (c == '\x1b' && Serial.available() >= 2) { 
                char b = Serial.read(); 
                char d = Serial.read(); 

                if (d == 'A') { 
                    tft.fillRect(racketX, racketY, RACKET_WIDTH, RACKET_LENGTH, ST77XX_BLACK);
                    racketY = std::max(0, racketY - 12);
                    tft.fillRect(racketX, racketY, RACKET_WIDTH, RACKET_LENGTH, colour);
                } else if (d == 'B') {
                    tft.fillRect(racketX, racketY, RACKET_WIDTH, RACKET_LENGTH, ST77XX_BLACK);
                    racketY = std::min(Y_MAX - RACKET_LENGTH, racketY + 12);
                    tft.fillRect(racketX, racketY, RACKET_WIDTH, RACKET_LENGTH, colour);
                }
            }
        }

        tft.fillCircle(x, y, ballRadius, ST77XX_BLACK);

        x += speedX;
        y += speedY;

        if (x - ballRadius <= racketX + RACKET_WIDTH &&
            y >= racketY &&
            y <= racketY + RACKET_LENGTH) {
            x = racketX + RACKET_WIDTH + ballRadius;
            speedX = -speedX;
            speedY += (std::rand() % 3 - 1); 
            speedY = std::max(-3, std::min(speedY, 3));
        }

        if (y - ballRadius <= 0 || y + ballRadius >= Y_MAX) {
            speedY = -speedY;
        }

        if (x + ballRadius >= X_MAX) {
            x = X_MAX - ballRadius; 
            speedX = -speedX;
        }

        if (x - ballRadius < 0) {
            x = X_MAX / 2;
            y = Y_MAX / 2;
            speedX = 1;
            speedY = 2;
        }
        if(optimisedBorderRedraw%10 ==0)  fillBorders();
        else optimisedBorderRedraw = (optimisedBorderRedraw+1) % 10;
        tft.fillCircle(x, y, ballRadius, colour);

        vTaskDelay(10 / portTICK_PERIOD_MS); 
    }
}