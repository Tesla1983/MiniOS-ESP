#ifndef PINGPONG_H
#define PINGPONG_H

#include <Arduino.h>
#include <Adafruit_ST7789.h>



extern bool screenLocked;


#define X_MAX 320
#define Y_MAX 230

#define RACKET_WIDTH 5
#define RACKET_LENGTH 30


void pingpongGame();

#endif 