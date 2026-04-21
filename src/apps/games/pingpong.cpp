#include "display.h"
#include "pingpong.h"
#include <cstdlib>
#include <time.h>
#include <algorithm> 

static void fillBorders(){
    tft.fillRect(5,0,310,5,ST77XX_BLUE);
    tft.fillRect(5,225,310,5,ST77XX_BLUE);
    tft.fillRect(315,0,5,230,ST77XX_RED);
}

static int optimisedBorderRedraw = 0;

typedef enum {
    PING_PONG_GAME_STATE_PLAYING,
    PING_PONG_GAME_STATE_LOST,
    PING_PONG_GAME_STATE_WAITING_RESTART
} GameState;

static void pingpongTask(void* pvParameters) {
     vTaskDelay(70 / portTICK_PERIOD_MS);
    int16_t score = 0;
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
    int speedY = 1;
    int ballRadius = 5;
    uint16_t colour = 65535;
    uint16_t ballColour = 65535;
    int16_t racketX = 0;
    int16_t racketY = Y_MAX / 2;
    tft.drawRect(racketX, racketY, RACKET_WIDTH, RACKET_LENGTH, colour);
    fillBorders();

    GameState state = PING_PONG_GAME_STATE_PLAYING;
    tft.setCursor(180, 230);
    tft.fillRect(180, 230, 140, 10, ST77XX_BLACK);
    tft.print("Score:");
    tft.print(score);

    while (true) {
        if (Serial.available()) {
            char c = Serial.read();

            if (c == '\n') {
                clearScreen();
                screenLocked = false;
                screenJustUnlocked = true;
                vTaskDelete(NULL);
                vTaskDelay(30/portTICK_PERIOD_MS);
                return;
            }

            if (state == PING_PONG_GAME_STATE_WAITING_RESTART && c == 'r') {
                tft.setCursor(X_MAX / 2 - 60, Y_MAX / 2);
                tft.setTextSize(1);
                tft.setTextColor(ST77XX_BLACK);
                tft.print("You Lost :(");
                tft.setCursor(X_MAX / 2 - 60, Y_MAX / 2 + 30);
                tft.setTextColor(ST77XX_BLACK);
                tft.println("Press 'R' to restart.");

                score = 0;
                ballColour = 65535;
                racketY = Y_MAX / 2;
                tft.drawRect(racketX, racketY, RACKET_WIDTH, RACKET_LENGTH, colour);
                fillBorders();

                tft.setCursor(180, 230);
                tft.fillRect(180, 230, 140, 10, ST77XX_BLACK);
                tft.setTextColor(ST77XX_WHITE);
                tft.print("Score:");
                tft.print(score);
                tft.setCursor(5, 230);
                tft.print("Press ENTER to exit...");

                state = PING_PONG_GAME_STATE_PLAYING;
            }

            if (state == PING_PONG_GAME_STATE_PLAYING && c == '\x1b' && Serial.available() >= 2) {
                char b = Serial.read();
                char d = Serial.read();

                if (d == 'A') {
                    tft.fillRect(racketX, racketY, RACKET_WIDTH, RACKET_LENGTH, ST77XX_BLACK);
                    racketY = std::max((int16_t)0, (int16_t)(racketY - 15));
                    tft.fillRect(racketX, racketY, RACKET_WIDTH, RACKET_LENGTH, colour);
                } else if (d == 'B') {
                    tft.fillRect(racketX, racketY, RACKET_WIDTH, RACKET_LENGTH, ST77XX_BLACK);
                    racketY = std::min((int16_t)(Y_MAX - RACKET_LENGTH), (int16_t)(racketY + 15));
                    tft.fillRect(racketX, racketY, RACKET_WIDTH, RACKET_LENGTH, colour);
                }
            }
        }

        if (state == PING_PONG_GAME_STATE_PLAYING) {
              

            tft.fillCircle(x, y, ballRadius, ST77XX_BLACK);

            x += speedX;
            y += speedY;

            if (x > 0 &&
                x - ballRadius <= racketX + RACKET_WIDTH &&
                y >= racketY &&
                y <= racketY + RACKET_LENGTH) {

                x = racketX + RACKET_WIDTH + ballRadius;
                speedX = -speedX;
                speedY += (std::rand() % 3 - 1);
                speedY = std::max(-2, std::min(speedY, 2));
                score++;
                if ( score == 5) ballColour = ST77XX_YELLOW;
                if ( score == 10) ballColour = ST77XX_ORANGE;
                if ( score == 15) ballColour = ST77XX_GREEN;
                if ( score == 20) ballColour = ST77XX_CYAN;
                tft.setCursor(180, 230);
                tft.fillRect(180, 230, 140, 10, ST77XX_BLACK);
                tft.print("Score:");
                tft.print(score);

            }

            if (y - ballRadius <= 0 || y + ballRadius >= Y_MAX) {
                speedY = -speedY;
            }

            if (x + ballRadius >= X_MAX) {
                x = X_MAX - ballRadius;
                speedX = -speedX;
            }

            if (x - ballRadius < 0) {
                tft.fillCircle(x, y, ballRadius, ST77XX_BLACK);
                tft.fillRect(racketX, racketY, RACKET_WIDTH, RACKET_LENGTH, ST77XX_BLACK);

                x = X_MAX / 2;
                y = Y_MAX / 2;
                speedX = ((std::rand() % 2) == 1) ? 1 : -1;
                speedY = ((std::rand() % 2) == 1) ? -2 : 2;

                tft.fillRect(180, 230, 140, 10, ST77XX_BLACK);
                tft.fillCircle(X_MAX/2, Y_MAX/2, ballRadius, ST77XX_BLACK);
                tft.setCursor(X_MAX / 2 - 60, Y_MAX / 2);
                tft.setTextSize(1);
                tft.setTextColor(ST77XX_RED);
                tft.println("You Lost :(");
                tft.setCursor(X_MAX / 2 - 60, Y_MAX / 2 + 30);
                tft.setTextColor(ST77XX_WHITE);
                tft.println("Press 'R' to restart.");

                optimisedBorderRedraw = 0;

                state = PING_PONG_GAME_STATE_LOST;


                vTaskDelay(10 / portTICK_PERIOD_MS);
                continue;
            }

            optimisedBorderRedraw = (optimisedBorderRedraw + 1) % 5;
            if (optimisedBorderRedraw == 0) fillBorders();

            tft.fillCircle(x, y, ballRadius, ballColour);

        } else if (state == PING_PONG_GAME_STATE_LOST) {
            state = PING_PONG_GAME_STATE_WAITING_RESTART;
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void pingpongGame() {
    xTaskCreate(
        pingpongTask,
        "PingPongTask",
        6144,           
        NULL,
        1,             
        NULL
    );
}