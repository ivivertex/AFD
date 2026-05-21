#include "LedIndicator.h"
#include "SystemStatus.h"
#include "LevelSensor.h"
#include "WiFiManager.h"
#include "PumpControl.h"
#include "Settings.h"

CRGB LedIndicator::leds[NUM_LEDS];
CRGB LedIndicator::targetColor = CRGB::Black;
LEDMode LedIndicator::mode = LEDMode::NORMAL;
unsigned long LedIndicator::lastBlink = 0;
bool LedIndicator::blinkState = false;
int LedIndicator::blinkInterval = 500;

uint8_t LedIndicator::hue = 0;
unsigned long LedIndicator::lastRainbowUpdate = 0;

uint8_t LedIndicator::brightness = DEFAULT_LED_BRIGHTNESS;

void LedIndicator::begin(uint8_t initialBrightness) {
  brightness = initialBrightness;
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(brightness);
  FastLED.show();
}

void LedIndicator::update() {
  updateState();
  applyColor();
}

void LedIndicator::updateState() {
  bool lowLevel = LevelSensor::isLowLevel();
  bool calibrating = SystemStatus::isCalibrating();
  bool pumpOn = (digitalRead(PUMP_PIN) == HIGH);
  bool apMode = isAPMode();

  static LEDMode lastMode = LEDMode::NORMAL;
  LEDMode newMode = LEDMode::NORMAL;

  if (lowLevel) {
    newMode = LEDMode::BLINK;
    targetColor = CRGB::Red;
    blinkInterval = 400;
  } else if (calibrating) {
    newMode = LEDMode::NORMAL;
    targetColor = CRGB(255, 128, 0); // оранжевый (замена жёлтого)
  } else if (pumpOn) {
    newMode = LEDMode::NORMAL;
    targetColor = CRGB::Blue;
  } else if (apMode) {
    newMode = LEDMode::RAINBOW;
  } else {
    newMode = LEDMode::NORMAL;
    targetColor = CRGB::Green;
  }

  if (newMode != lastMode) {
    lastMode = newMode;
    if (newMode == LEDMode::BLINK) {
      lastBlink = millis();
      blinkState = false;
    }
    #ifdef DEBUG_ENABLED
    Serial.print(F("LED mode changed to: "));
    switch (newMode) {
      case LEDMode::BLINK: Serial.println(F("RED BLINK (low level)")); break;
      case LEDMode::NORMAL:
        if (calibrating) Serial.println(F("ORANGE SOLID (calibrating)"));
        else if (pumpOn) Serial.println(F("BLUE SOLID (pump on)"));
        else Serial.println(F("GREEN SOLID (normal)"));
        break;
      case LEDMode::RAINBOW: Serial.println(F("RAINBOW (AP mode)")); break;
    }
    #endif
  }
  mode = newMode;
}

void LedIndicator::applyColor() {
  if (mode == LEDMode::RAINBOW) {
    if (millis() - lastRainbowUpdate > 30) {
      lastRainbowUpdate = millis();
      hue += 2;
      leds[0] = CHSV(hue, 255, 255);
      FastLED.show();
    }
  } else if (mode == LEDMode::BLINK) {
    if (millis() - lastBlink > blinkInterval) {
      lastBlink = millis();
      blinkState = !blinkState;
      leds[0] = blinkState ? targetColor : CRGB::Black;
      FastLED.show();
    }
  } else if (mode == LEDMode::NORMAL) {
    leds[0] = targetColor;
    FastLED.show();
  }
}

void LedIndicator::setBrightness(uint8_t value) {
  brightness = value;
  FastLED.setBrightness(brightness);
  FastLED.show();
}