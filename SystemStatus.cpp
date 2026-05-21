#include "SystemStatus.h"
#include "Settings.h"

bool SystemStatus::_calibrating = false;
unsigned long SystemStatus::_calibrationStartTime = 0;
float SystemStatus::_calibrationTargetVolume = 0.0f;
bool SystemStatus::_fixedCalibration = false;
unsigned long SystemStatus::_fixedCalibrationElapsed = 0;
unsigned long SystemStatus::_fixedCalibrationEndTime = 0;
unsigned long SystemStatus::_pendingFixedTime = 0;

void SystemStatus::begin() {
  _calibrating = false;
  _calibrationStartTime = 0;
  _calibrationTargetVolume = 0.0f;
  _fixedCalibration = false;
  _fixedCalibrationElapsed = 0;
  _fixedCalibrationEndTime = 0;
  _pendingFixedTime = 0;
}

bool SystemStatus::isCalibrating() {
  return _calibrating;
}

void SystemStatus::setCalibrating(bool state) {
  _calibrating = state;
}

unsigned long SystemStatus::getCalibrationStart() {
  return _calibrationStartTime;
}

void SystemStatus::setCalibrationStart(unsigned long micros) {
  _calibrationStartTime = micros;
}

float SystemStatus::getCalibrationTarget() {
  return _calibrationTargetVolume;
}

void SystemStatus::setCalibrationTarget(float volume) {
  _calibrationTargetVolume = volume;
}

void SystemStatus::resetCalibration() {
  _calibrating = false;
  _calibrationStartTime = 0;
  _calibrationTargetVolume = 0.0f;
  _fixedCalibration = false;
  _fixedCalibrationElapsed = 0;
  _fixedCalibrationEndTime = 0;
  // _pendingFixedTime не сбрасываем, т.к. оно может понадобиться для сохранения
}

bool SystemStatus::isFixedCalibration() {
  return _fixedCalibration;
}

void SystemStatus::setFixedCalibration(bool fixed) {
  _fixedCalibration = fixed;
}

unsigned long SystemStatus::getFixedCalibrationElapsed() {
  return _fixedCalibrationElapsed;
}

void SystemStatus::setFixedCalibrationElapsed(unsigned long seconds) {
  _fixedCalibrationElapsed = seconds;
}

void SystemStatus::startFixedCalibration(unsigned long durationSeconds) {
  _fixedCalibration = true;
  _fixedCalibrationEndTime = millis() + durationSeconds * 1000UL;
  _fixedCalibrationElapsed = 0;
  _pendingFixedTime = durationSeconds;          // сохраняем время для возможного сохранения
  setCalibrating(true);
  setCalibrationStart(micros());
  setCalibrationTarget(0);
}

void SystemStatus::stopFixedCalibration() {
  if (_fixedCalibration) {
    _fixedCalibration = false;
    _fixedCalibrationEndTime = 0;
    _fixedCalibrationElapsed = 0;
    setCalibrating(false);
    // _pendingFixedTime оставляем – пользователь сможет сохранить объём позже
  }
}

void SystemStatus::cancelFixedCalibration() {
  _fixedCalibration = false;
  _fixedCalibrationEndTime = 0;
  _fixedCalibrationElapsed = 0;
  _pendingFixedTime = 0;
  resetCalibration();
}

unsigned long SystemStatus::getFixedCalibrationRemaining() {
  if (!_fixedCalibration || _fixedCalibrationEndTime == 0) return 0;
  unsigned long now = millis();
  if (now >= _fixedCalibrationEndTime) return 0;
  return (_fixedCalibrationEndTime - now) / 1000;
}

bool SystemStatus::update() {
  if (_fixedCalibration && _fixedCalibrationEndTime > 0 && millis() >= _fixedCalibrationEndTime) {
    // Калибровка завершилась по таймауту, останавливаем насос, но не сбрасываем _pendingFixedTime
    _fixedCalibration = false;
    _fixedCalibrationEndTime = 0;
    _calibrating = false;
    return true;
  }
  return false;
}

void SystemStatus::setPendingFixedTime(unsigned long seconds) {
  _pendingFixedTime = seconds;
}

unsigned long SystemStatus::getPendingFixedTime() {
  return _pendingFixedTime;
}

void SystemStatus::clearPendingFixedTime() {
  _pendingFixedTime = 0;
}