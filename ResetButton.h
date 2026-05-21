#ifndef RESETBUTTON_H
#define RESETBUTTON_H

#include <Arduino.h>
#include "Settings.h"

class ResetButton {
public:
  static void begin(int pin, ResetButtonActive activeLevel = ResetButtonActive::ACTIVE_LOW);
  static void update();
  static bool isResetRequested();
  static bool isFactoryResetRequested();

private:
  static int _pin;
  static ResetButtonActive _activeLevel;
  static unsigned long _pressStartTime;
  static bool _wasPressed;
  static bool _rebootRequested;
  static bool _factoryResetRequested;
  static const unsigned long REBOOT_HOLD_TIME;
  static const unsigned long FACTORY_HOLD_TIME;
};

#endif