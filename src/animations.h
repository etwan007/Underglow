#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include <FastLED.h>

extern CRGB leds[]; // Declare the LEDs array as external
extern const int NUM_LEDS; // Declare NUM_LEDS as external

void rainbow_wave();
void confetti();
void sinelon();
void bpm();
void juggle();
void handleAnimation(int animationIndex);

#endif
