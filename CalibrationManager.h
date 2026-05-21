#ifndef CALIBRATIONMANAGER_H
#define CALIBRATIONMANAGER_H

#include <Arduino.h>

class CalibrationManager {
public:
  static float getFlowRate();
  static void setFlowRate(float rate);
  static bool load();
  static bool save();
  static bool isCalibrated();

  // Для калибровки из MQTT
  static float getTargetVolume();
  static void setTargetVolume(float volume);

private:
  static float flowRate;
  static float targetVolume; // последний установленный объём калибровки
  static const char* filename;
};

#endif