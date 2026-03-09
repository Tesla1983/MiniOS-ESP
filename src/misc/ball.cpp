#include "display.h"
#include "ball.h"
#include <cstdlib>
#include <time.h>

void initialiseBall(int ballRadius, bool trail) {

    std::srand(time(nullptr));

    tft.fillScreen(ST77XX_BLACK);
    screenLocked = true;
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.setTextSize(1);
    tft.setCursor(5, 230);
    tft.print("Press ENTER to exit...");
    Serial.println("Press ENTER to exit...");

    int x = 160;
    int y = 115;
    int speedX = 2;  
    int speedY = 2;  
    uint64_t colour = 65535;
    
    while (true) {
        if (Serial.available()) {
            char c = Serial.read();
            if (c == '\n') {
                clearScreen();
                screenLocked = false;
                break;
            }
        }
        if (!trail) tft.fillCircle(x, y, ballRadius, ST77XX_BLACK);
        x += speedX;
        y += speedY;
        if (x - ballRadius <= 1 || x + ballRadius >= X_MAX){ 
            speedX = -speedX;
            colour = std::rand()%65535;
        }
        if (y - ballRadius <= 1 || y + ballRadius >= Y_MAX){ 
            speedY = -speedY;
            colour = std::rand()%65535;
        }
        tft.fillCircle(x, y, ballRadius, colour);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}