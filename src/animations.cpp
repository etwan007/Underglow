#include <FastLED.h>
#include "animations.h"

extern CRGB leds[]; // Ensure the global LEDs array is declared
extern const int NUM_LEDS; // Ensure NUM_LEDS is declared
uint8_t gHue = 0;

void rainbow_wave() {
  uint8_t thisHue = beat8(10, 255);  // A simple rainbow march.
  fill_rainbow(leds, NUM_LEDS, thisHue, 10);  // Use FastLED's fill_rainbow routine.
  FastLED.show();
}

void confetti() {
  fadeToBlackBy(leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV(gHue + random8(64), 200, 255);
  FastLED.show();
}

void sinelon() {
  fadeToBlackBy(leds, NUM_LEDS, 20);
  int pos = beatsin16(13, 0, NUM_LEDS - 1);
  leds[pos] += CHSV(gHue, 255, 192);
  FastLED.show();
}

void bpm() {
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
  FastLED.show();
}

void juggle() {
  fadeToBlackBy(leds, NUM_LEDS, 20);
  uint8_t dothue = 0;
  for (int i = 0; i < 8; i++) {
    leds[beatsin16(i + 7, 0, NUM_LEDS - 1)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
  FastLED.show();
}

void handleAnimation(int animationIndex) {
  switch (animationIndex) {
    case 1:
      rainbow_wave();
      break;
    case 2:
      confetti();
      break;
    case 3:
      sinelon();
      break;
    case 4:
      bpm();
      break;
    case 5:
      juggle();
      break;
    default:
      break;
  }
}
