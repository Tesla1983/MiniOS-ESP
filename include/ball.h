#ifndef BALL_H
#define BALL_H

#include <Arduino.h>
#include <Adafruit_ST7789.h>

#define Y_MAX 230
#define X_MAX 320

extern bool screenLocked;
void initialiseBall(int ballRadius, bool trail);

#endif