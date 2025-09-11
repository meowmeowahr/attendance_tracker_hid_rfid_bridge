#ifndef LED_H
#define LED_H

#include <Adafruit_NeoPixel.h>

#define COLOR_INIT Adafruit_NeoPixel::Color(100, 100, 100)
#define COLOR_IDLE Adafruit_NeoPixel::Color(0, 0, 200)
#define COLOR_SENDING Adafruit_NeoPixel::Color(0, 200, 0)

#define PIXEL_FREQ NEO_KHZ800
#define PIXEL_ORDER NEO_GRB
#define PIXEL_CNT 1

#endif // LED_Hs