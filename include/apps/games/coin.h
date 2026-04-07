#ifndef COIN_H
#define COIN_H

#include <Arduino.h>
#include <Adafruit_ST7789.h>

typedef enum{
    HEAD,
    TAIL
}CoinState;

extern bool screenLocked;
#define X_MAX 320
#define Y_MAX 230
void coinGame();

#endif