#include "ButtonManager.h"
#include "Settings.h"

int ButtonManager::_pin = -1;
ButtonActive ButtonManager::_activeLevel = ButtonActive::ACTIVE_HIGH;
bool ButtonManager::_lastState = false;
bool ButtonManager::_pressedFlag = false;
bool ButtonManager::_debouncedState = false;
unsigned long ButtonManager::_lastDebounceTime = 0;

void ButtonManager::begin(int pin, ButtonActive activeLevel) {
  _pin = pin;
  _activeLevel = activeLevel;
  
  // Настройка подтягивающего резистора
  if (_activeLevel == ButtonActive::ACTIVE_LOW) {
    pinMode(_pin, INPUT_PULLUP);
  } else {
    pinMode(_pin, INPUT);
  }
  
  _lastState = digitalRead(_pin);
  _pressedFlag = false;
  _debouncedState = _lastState;
  _lastDebounceTime = 0;
  #ifdef DEBUG_ENABLED
  Serial.print(F("Button initialized on pin "));
  Serial.print(_pin);
  Serial.print(F(" active level: "));
  Serial.println(static_cast<uint8_t>(_activeLevel));
  #endif
}

void ButtonManager::update() {
  if (_pin == -1) return;
  
  bool current = digitalRead(_pin);
  if (current != _debouncedState) {
    _lastDebounceTime = millis();
    _debouncedState = current;
  }
  
  if ((millis() - _lastDebounceTime) > BUTTON_DEBOUNCE_DELAY) {
    if (_debouncedState != _lastState) {
      _lastState = _debouncedState;
      if (_lastState == static_cast<uint8_t>(_activeLevel)) {
        _pressedFlag = true;
        #ifdef DEBUG_ENABLED
        Serial.println(F("Button press detected (debounced)"));
        #endif
      } else {
        #ifdef DEBUG_ENABLED
        Serial.println(F("Button release detected"));
        #endif
      }
    }
  }
}

bool ButtonManager::wasPressed() {
  if (_pressedFlag) {
    _pressedFlag = false;
    return true;
  }
  return false;
}