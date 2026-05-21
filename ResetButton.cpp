#include "ResetButton.h"
#include "Settings.h"

int ResetButton::_pin = -1;
ResetButtonActive ResetButton::_activeLevel = ResetButtonActive::ACTIVE_LOW;
unsigned long ResetButton::_pressStartTime = 0;
bool ResetButton::_wasPressed = false;
bool ResetButton::_rebootRequested = false;
bool ResetButton::_factoryResetRequested = false;
const unsigned long ResetButton::REBOOT_HOLD_TIME = 3000;
const unsigned long ResetButton::FACTORY_HOLD_TIME = 10000;

void ResetButton::begin(int pin, ResetButtonActive activeLevel) {
  _pin = pin;
  _activeLevel = activeLevel;
  pinMode(_pin, INPUT_PULLUP);
  _pressStartTime = 0;
  _wasPressed = false;
  _rebootRequested = false;
  _factoryResetRequested = false;
}

void ResetButton::update() {
  if (_pin == -1) return;
  if (_factoryResetRequested) return; // уже идёт factory reset

  bool current = digitalRead(_pin);
  bool isPressed = (current == static_cast<uint8_t>(_activeLevel));

  if (isPressed) {
    if (!_wasPressed) {
      _wasPressed = true;
      _pressStartTime = millis();
      #ifdef DEBUG_ENABLED
      Serial.println(F("Reset button pressed"));
      #endif
    } else {
      unsigned long duration = millis() - _pressStartTime;
      if (duration >= FACTORY_HOLD_TIME && !_factoryResetRequested) {
        _factoryResetRequested = true;
        _rebootRequested = false; // отменяем reboot, если он был
        #ifdef DEBUG_ENABLED
        Serial.println(F("Reset button factory reset requested (10s)"));
        #endif
      }
    }
  } else {
    // Кнопка отпущена
    if (_wasPressed) {
      _wasPressed = false;
      unsigned long duration = millis() - _pressStartTime;
      #ifdef DEBUG_ENABLED
      Serial.printf_P(PSTR("Reset button released, hold time = %lu ms\n"), duration);
      #endif
      if (duration >= REBOOT_HOLD_TIME && duration < FACTORY_HOLD_TIME) {
        _rebootRequested = true;
        #ifdef DEBUG_ENABLED
        Serial.println(F("Reset button reboot requested (3-10s)"));
        #endif
      }
      _pressStartTime = 0;
    }
  }
}

bool ResetButton::isResetRequested() {
  if (_rebootRequested) {
    _rebootRequested = false;
    return true;
  }
  return false;
}

bool ResetButton::isFactoryResetRequested() {
  if (_factoryResetRequested) {
    _factoryResetRequested = false;
    // также сбрасываем остальные флаги
    _rebootRequested = false;
    _wasPressed = false;
    _pressStartTime = 0;
    return true;
  }
  return false;
}