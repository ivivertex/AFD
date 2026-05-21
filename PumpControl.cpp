#include "PumpControl.h"
#include "Settings.h"
#include "MQTTManager.h"

static bool pumpState = false;

void initPump() {
  pinMode(PUMP_PIN, OUTPUT);
  pumpOff();
}

void pumpOn() {
  digitalWrite(PUMP_PIN, HIGH);
  pumpState = true;
  MQTTManager::publishState(true);
}

void pumpOff() {
  digitalWrite(PUMP_PIN, LOW);
  pumpState = false;
  MQTTManager::publishState(false);
}

bool isPumpOn() {
  return pumpState;
}