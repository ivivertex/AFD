#ifndef SYSTEMSTATUS_H
#define SYSTEMSTATUS_H

#include <Arduino.h>
#include "Settings.h"

class SystemStatus {
public:
  static void begin();
  static bool isCalibrating();
  static void setCalibrating(bool state);
  static unsigned long getCalibrationStart();
  static void setCalibrationStart(unsigned long micros);
  static float getCalibrationTarget();
  static void setCalibrationTarget(float volume);
  static void resetCalibration();

  // Фиксированная калибровка
  static bool isFixedCalibration();
  static void setFixedCalibration(bool fixed);
  static unsigned long getFixedCalibrationElapsed();
  static void setFixedCalibrationElapsed(unsigned long seconds);
  static void startFixedCalibration(unsigned long durationSeconds);
  static void stopFixedCalibration();           // остановить без сохранения, но сохранить время
  static void cancelFixedCalibration();         // полный сброс
  static unsigned long getFixedCalibrationRemaining();
  static bool update();  // возвращает true, если фиксированная калибровка завершилась по таймауту

  // Время фиксированной калибровки (сохраняется до сохранения объёма)
  static void setPendingFixedTime(unsigned long seconds);
  static unsigned long getPendingFixedTime();
  static void clearPendingFixedTime();

private:
  static bool _calibrating;
  static unsigned long _calibrationStartTime;
  static float _calibrationTargetVolume;
  static bool _fixedCalibration;
  static unsigned long _fixedCalibrationElapsed;
  static unsigned long _fixedCalibrationEndTime;
  static unsigned long _pendingFixedTime;      // время фиксированной калибровки, ожидающее сохранения
};

#endif