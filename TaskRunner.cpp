#include "TaskRunner.h"
#include "PumpControl.h"
#include "SystemStatus.h"
#include "MQTTManager.h"
#include "StartGuard.h"

extern void broadcastStatus();

bool TaskRunner::running = false;
unsigned long TaskRunner::stopTime = 0;
unsigned long TaskRunner::startTime = 0;
unsigned long TaskRunner::durationMs = 0;
int TaskRunner::currentVolume = 0;

void TaskRunner::start(unsigned long durationMs, int volume) {
  String error;
  if (!StartGuard::canStart(StartMode::DISPENSE, error)) {
    #ifdef DEBUG_ENABLED
    Serial.print(F("TaskRunner start blocked: "));
    Serial.println(error);
    #endif
    return;
  }
  
  if (running) stop();
  pumpOn();
  startTime = millis();
  TaskRunner::durationMs = durationMs;
  TaskRunner::currentVolume = volume;
  stopTime = startTime + durationMs;
  running = true;
  broadcastStatus();
}

void TaskRunner::stop() {
  if (running) {
    unsigned long elapsed = millis() - startTime;
    pumpOff();
    MQTTManager::publishDispenseFinished(false, currentVolume, elapsed);
  }
  running = false;
  startTime = 0;
  durationMs = 0;
  currentVolume = 0;
  broadcastStatus();
}

bool TaskRunner::isRunning() {
  return running;
}

void TaskRunner::update() {
  if (running && millis() >= stopTime) {
    pumpOff();
    unsigned long elapsed = millis() - startTime;
    MQTTManager::publishDispenseFinished(true, currentVolume, elapsed);
    running = false;
    startTime = 0;
    durationMs = 0;
    currentVolume = 0;
    broadcastStatus();
  }
}

unsigned long TaskRunner::getStartTime() {
  return startTime;
}

unsigned long TaskRunner::getDuration() {
  return durationMs;
}

int TaskRunner::getCurrentVolume() {
  return currentVolume;
}