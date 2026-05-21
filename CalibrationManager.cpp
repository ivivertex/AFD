#include "CalibrationManager.h"
#include "Settings.h"
#include "ConfigManager.h"   // для hasEnoughSpace
#include <LittleFS.h>
#include <ArduinoJson.h>

float CalibrationManager::flowRate = 0.0f;
float CalibrationManager::targetVolume = 100.0f;
const char* CalibrationManager::filename = "/calibration.json";

float CalibrationManager::getFlowRate() {
  return flowRate;
}

void CalibrationManager::setFlowRate(float rate) {
  flowRate = rate;
  save();
}

bool CalibrationManager::load() {
  if (!LittleFS.exists(filename)) {
    flowRate = 0.0f;
    return false;
  }
  File file = LittleFS.open(filename, "r");
  if (!file) return false;
  StaticJsonDocument<128> doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  if (error) return false;
  flowRate = doc["flowRate"] | 0.0f;
  targetVolume = doc["targetVolume"] | 100.0f;
  return true;
}

bool CalibrationManager::save() {
  StaticJsonDocument<128> doc;
  doc["flowRate"] = flowRate;
  doc["targetVolume"] = targetVolume;
  size_t jsonSize = measureJson(doc);
  
  // Проверка свободного места (используем общую функцию из ConfigManager)
  if (!hasEnoughSpace(jsonSize)) {
    #ifdef DEBUG_ENABLED
    Serial.println(F("Calibration save failed: insufficient space"));
    #endif
    return false;
  }
  
  File file = LittleFS.open(filename, "w");
  if (!file) return false;
  if (serializeJson(doc, file) == 0) {
    file.close();
    return false;
  }
  file.close();
  return true;
}

bool CalibrationManager::isCalibrated() {
  return flowRate > 0.0f;
}

float CalibrationManager::getTargetVolume() {
  return targetVolume;
}

void CalibrationManager::setTargetVolume(float volume) {
  targetVolume = volume;
  save();
}