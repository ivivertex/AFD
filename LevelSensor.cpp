#include "LevelSensor.h"
#include "Settings.h"

int LevelSensor::_pin = -1;
LevelActive LevelSensor::_activeLevel = LevelActive::ACTIVE_HIGH;
bool LevelSensor::_lastStableState = false;
bool LevelSensor::_currentState = false;
unsigned long LevelSensor::_lastDebounceTime = 0;
bool LevelSensor::_lastReading = false;
const unsigned long LevelSensor::DEBOUNCE_DELAY = 50;

void LevelSensor::begin(int pin, LevelActive activeLevel) {
  _pin = pin;
  _activeLevel = activeLevel;
  pinMode(_pin, INPUT_PULLUP);
  _lastReading = digitalRead(_pin);
  _lastStableState = _lastReading;
  _currentState = (_lastStableState == static_cast<uint8_t>(_activeLevel));
  #ifdef DEBUG_ENABLED
  Serial.print(F("Level sensor initialized on pin "));
  Serial.print(_pin);
  Serial.print(F(" active level: "));
  Serial.println(static_cast<uint8_t>(_activeLevel));
  Serial.print(F("Initial state: "));
  Serial.println(_currentState ? F("OK") : F("LOW"));
  #endif
}

void LevelSensor::update() {
  if (_pin == -1) return;
  
  bool reading = digitalRead(_pin);
  
  #ifdef DEBUG_ENABLED
  static unsigned long lastDebugTime = 0;
  // Уменьшаем частоту до 10 секунд (пункт 6.2)
  if (millis() - lastDebugTime >= 10000) {
    lastDebugTime = millis();
    Serial.print(F("[LevelSensor] Pin reads: "));
    Serial.print(reading);
    Serial.print(F(" (active level: "));
    Serial.print(static_cast<uint8_t>(_activeLevel));
    Serial.print(F(") -> "));
    Serial.println(reading == static_cast<uint8_t>(_activeLevel) ? F("OK") : F("LOW"));
  }
  #endif

  if (reading != _lastReading) {
    _lastDebounceTime = millis();
    _lastReading = reading;
  }

  if ((millis() - _lastDebounceTime) >= DEBOUNCE_DELAY) {
    if (reading != _lastStableState) {
      _lastStableState = reading;
      _currentState = (reading == static_cast<uint8_t>(_activeLevel));
      #ifdef DEBUG_ENABLED
      Serial.print(F("Level status changed: "));
      Serial.println(_currentState ? F("OK") : F("LOW"));
      #endif
    }
  }
}

bool LevelSensor::isLevelOk() {
  return _currentState;
}

bool LevelSensor::isLowLevel() {
  return !_currentState;
}