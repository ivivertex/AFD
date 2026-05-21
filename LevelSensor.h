#ifndef LEVELSENSOR_H
#define LEVELSENSOR_H

#include <Arduino.h>
#include "Settings.h"

class LevelSensor {
public:
  static void begin(int pin, LevelActive activeLevel = LevelActive::ACTIVE_HIGH);
  static void update();
  static bool isLevelOk();
  static bool isLowLevel();

private:
  static int _pin;
  static LevelActive _activeLevel;
  static bool _lastStableState;
  static bool _currentState;
  static unsigned long _lastDebounceTime;
  static bool _lastReading;
  static const unsigned long DEBOUNCE_DELAY;
};

#endif