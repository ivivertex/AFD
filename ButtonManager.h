#ifndef BUTTONMANAGER_H
#define BUTTONMANAGER_H

#include <Arduino.h>
#include "Settings.h"

class ButtonManager {
public:
  static void begin(int pin, ButtonActive activeLevel = ButtonActive::ACTIVE_HIGH);
  static void update();
  static bool wasPressed();

private:
  static int _pin;
  static ButtonActive _activeLevel;
  static bool _lastState;
  static bool _pressedFlag;
  static bool _debouncedState;      // добавлено
  static unsigned long _lastDebounceTime; // добавлено
};

#endif