#ifndef LEDINDICATOR_H
#define LEDINDICATOR_H

#include <Arduino.h>
#include <FastLED.h>
#include "Settings.h"

#define LED_PIN D3
#define NUM_LEDS 1

class LedIndicator {
public:
  static void begin(uint8_t initialBrightness = DEFAULT_LED_BRIGHTNESS);
  static void update();
  static void setBrightness(uint8_t value);

private:
  static void updateState();
  static void applyColor();

  static CRGB leds[NUM_LEDS];
  static CRGB targetColor;
  static LEDMode mode;
  static unsigned long lastBlink;
  static bool blinkState;
  static int blinkInterval;

  static uint8_t hue;
  static unsigned long lastRainbowUpdate;

  static uint8_t brightness;
};

#endif