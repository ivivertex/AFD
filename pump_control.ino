#include <Arduino.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include "ConfigManager.h"
#include "WiFiManager.h"
#include "WebServer.h"
#include "PumpControl.h"
#include "CalibrationManager.h"
#include "SchedulerManager.h"
#include "TaskRunner.h"
#include "RTCManager.h"
#include "DisplayManager.h"
#include "TimeManager.h"
#include "ButtonManager.h"
#include "LevelSensor.h"
#include "MQTTManager.h"
#include "SystemStatus.h"
#include "Settings.h"
#include "ResetButton.h"
#include "LedIndicator.h"
#include "version.h"

WebSocketsServer webSocket(81);

const unsigned long STATUS_BROADCAST_INTERVAL = 1000;
unsigned long lastStatusBroadcast = 0;

void broadcastStatus() {
  #ifdef DEBUG_ENABLED
  Serial.println(F("broadcastStatus() called"));
  #endif
  StaticJsonDocument<384> doc;
  doc["pump"] = isPumpOn() ? "ON" : "OFF";
  doc["levelOk"] = LevelSensor::isLevelOk();
  doc["mqtt"] = MQTTManager::isConnected() ? "connected" : "disconnected";
  doc["calibrated"] = CalibrationManager::isCalibrated();
  doc["calibrating"] = SystemStatus::isCalibrating();
  doc["time"] = TimeManager::getTimeString();
  doc["date"] = TimeManager::getDateString();

  bool dispenseActive = TaskRunner::isRunning();
  doc["dispenseActive"] = dispenseActive;
  if (dispenseActive) {
    unsigned long elapsed = millis() - TaskRunner::getStartTime();
    unsigned long total = TaskRunner::getDuration();
    doc["dispenseElapsed"] = elapsed;
    doc["dispenseTotal"] = total;
  }

  String json;
  serializeJson(doc, json);
  #ifdef DEBUG_ENABLED
  Serial.print(F("Broadcasting JSON: "));
  Serial.println(json);
  #endif
  webSocket.broadcastTXT(json);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      #ifdef DEBUG_ENABLED
      Serial.printf_P(PSTR("[%u] Disconnected!\n"), num);
      #endif
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        #ifdef DEBUG_ENABLED
        Serial.printf_P(PSTR("[%u] Connected from %d.%d.%d.%d\n"), num, ip[0], ip[1], ip[2], ip[3]);
        #endif
        broadcastStatus();
      }
      break;
    case WStype_TEXT:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  #ifdef DEBUG_ENABLED
  Serial.print(F("\n\n=== Pump Control Starting (V"));
  Serial.print(VERSION);
  Serial.println(F(") ==="));
  #endif

  ESP.wdtEnable(8000000);

  if (!initConfig()) {
    Serial.println(F("Failed to mount LittleFS"));
  }

  SystemStatus::begin();
  CalibrationManager::load();
  SchedulerManager::load();
  loadConfig();
  loadMQTTConfig();

  initPump();
  ButtonManager::begin(BUTTON_PIN, ButtonActive::ACTIVE_HIGH);
  
  DeviceConfig devCfg;
  loadDeviceConfig(devCfg);
  LedIndicator::begin(devCfg.ledBrightness);
  LevelSensor::begin(LEVEL_PIN, devCfg.levelActiveLevel);
  
  ResetButton::begin(RESET_BUTTON_PIN, ResetButtonActive::ACTIVE_LOW);

  if (!RTCManager::begin()) {
    Serial.println(F("RTC initialization failed"));
  }

  DisplayManager::begin();

  if (isWiFiConfigValid()) {
    if (connectToWiFi()) {
      #ifdef DEBUG_ENABLED
      Serial.println(F("Syncing time via NTP..."));
      #endif
      TimeManager::syncNTP();
      MQTTConfig* mqtt = getMQTTConfig();
      if (strlen(mqtt->server) > 0) {
        MQTTManager::begin(mqtt);
      }
    } else {
      startAPMode();
    }
  } else {
    startAPMode();
  }

  startWebServer();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  DisplayManager::setIP(getDisplayIP());
  DisplayManager::setTime(RTCManager::getTimeString());
  DisplayManager::setDate(RTCManager::getShortDateString());
}

void loop() {
  LedIndicator::update();

  if (SystemStatus::update()) {
    pumpOff();
    SystemStatus::resetCalibration();
    MQTTManager::publishCalibrationStatus(false, 0, 0, CalibrationManager::isCalibrated());
    broadcastStatus();
  }

  handleWebServer();
  webSocket.loop();
  MQTTManager::loop();
  ESP.wdtFeed();

  ResetButton::update();
  if (ResetButton::isFactoryResetRequested()) {
    #ifdef DEBUG_ENABLED
    Serial.println(F("Factory reset requested - clearing all config and restarting..."));
    #endif
    clearConfig();
    delay(100);
    ESP.restart();
  } else if (ResetButton::isResetRequested()) {
    #ifdef DEBUG_ENABLED
    Serial.println(F("Reset requested - restarting without config clear..."));
    #endif
    delay(100);
    ESP.restart();
  }

  static unsigned long wifiLostTime = 0;
  if (!isWiFiConnected() && !isAPMode()) {
    if (wifiLostTime == 0) {
      wifiLostTime = millis();
      #ifdef DEBUG_ENABLED
      Serial.println(F("WiFi connection lost, waiting 30s before switching to AP..."));
      #endif
    } else if (millis() - wifiLostTime > WIFI_LOST_TIMEOUT) {
      #ifdef DEBUG_ENABLED
      Serial.println(F("No WiFi for 30s, switching to AP mode"));
      #endif
      startAPMode();
      wifiLostTime = 0;
    }
  } else {
    wifiLostTime = 0;
  }

  static unsigned long lastWifiRetry = 0;
  if (isAPMode() && isWiFiConfigValid()) {
    if (millis() - lastWifiRetry >= WIFI_RETRY_INTERVAL) {
      lastWifiRetry = millis();
      #ifdef DEBUG_ENABLED
      Serial.println(F("AP mode: attempting to reconnect to WiFi..."));
      #endif
      if (connectToWiFi()) {
        #ifdef DEBUG_ENABLED
        Serial.println(F("Switched back to STA mode"));
        #endif
        TimeManager::syncNTP();
        MQTTConfig* mqtt = getMQTTConfig();
        if (strlen(mqtt->server) > 0) {
          MQTTManager::begin(mqtt);
        }
      } else {
        #ifdef DEBUG_ENABLED
        Serial.println(F("Reconnection failed, staying in AP mode"));
        #endif
      }
    }
  } else {
    lastWifiRetry = millis();
  }

  LevelSensor::update();

  static bool lastLevelOk = LevelSensor::isLevelOk();
  bool currentLevelOk = LevelSensor::isLevelOk();
  #ifdef DEBUG_ENABLED
  static unsigned long lastLevelPrint = 0;
  if (millis() - lastLevelPrint > 1000) {
    lastLevelPrint = millis();
    Serial.print(F("Level check: lastLevelOk="));
    Serial.print(lastLevelOk);
    Serial.print(F(", currentLevelOk="));
    Serial.println(currentLevelOk);
  }
  #endif

  if (currentLevelOk != lastLevelOk) {
    lastLevelOk = currentLevelOk;
    broadcastStatus();
    MQTTManager::publishLevel(currentLevelOk);
    #ifdef DEBUG_ENABLED
    Serial.print(F("*** Level changed! New level OK: "));
    Serial.println(currentLevelOk);
    #endif
  }

  if (LevelSensor::isLowLevel()) {
    if (isPumpOn()) {
      pumpOff();
      if (!TaskRunner::isRunning()) {
        broadcastStatus();
      }
    }
    if (SystemStatus::isCalibrating()) {
      SystemStatus::resetCalibration();
      MQTTManager::publishCalibrationStatus(false, 0, 0, CalibrationManager::isCalibrated());
      broadcastStatus();
    }
    if (TaskRunner::isRunning()) {
      TaskRunner::stop();
    }
  }

  ButtonManager::update();
  if (ButtonManager::wasPressed()) {
    #ifdef DEBUG_ENABLED
    Serial.println(F("Button pressed"));
    #endif
    if (LevelSensor::isLowLevel()) {
      #ifdef DEBUG_ENABLED
      Serial.println(F("Button ignored - low level"));
      #endif
    } else {
      if (isPumpOn()) {
        pumpOff();
        if (SystemStatus::isCalibrating()) {
          SystemStatus::resetCalibration();
          MQTTManager::publishCalibrationStatus(false, 0, 0, CalibrationManager::isCalibrated());
        }
        if (TaskRunner::isRunning()) {
          TaskRunner::stop();
        } else {
          broadcastStatus();
        }
      } else {
        pumpOn();
        broadcastStatus();
      }
    }
  }

  TaskRunner::update();

  if (TaskRunner::isRunning()) {
    unsigned long now = millis();
    if (now - lastStatusBroadcast >= STATUS_BROADCAST_INTERVAL) {
      lastStatusBroadcast = now;
      broadcastStatus();
    }
  }

  if (SystemStatus::isCalibrating() && !SystemStatus::isFixedCalibration()) {
    if (micros() - SystemStatus::getCalibrationStart() > CALIBRATION_TIMEOUT_US) {
      pumpOff();
      SystemStatus::resetCalibration();
      MQTTManager::publishCalibrationStatus(false, 0, 0, CalibrationManager::isCalibrated());
      broadcastStatus();
      #ifdef DEBUG_ENABLED
      Serial.println(F("Calibration timeout - stopped"));
      #endif
    } else {
      static unsigned long lastCalStatus = 0;
      if (millis() - lastCalStatus >= 1000) {
        lastCalStatus = millis();
        float elapsed = (micros() - SystemStatus::getCalibrationStart()) / 1000000.0f;
        MQTTManager::publishCalibrationStatus(true, SystemStatus::getCalibrationTarget(), elapsed, CalibrationManager::isCalibrated());
        broadcastStatus();
      }
    }
  }

  static unsigned long lastSchedulerCheck = 0;
  if (millis() - lastSchedulerCheck >= SCHEDULER_CHECK_INTERVAL) {
    lastSchedulerCheck = millis();
    SchedulerManager::checkAndRun();
  }

  static unsigned long lastDisplayUpdate = 0;
  if (millis() - lastDisplayUpdate >= 500) {
    lastDisplayUpdate = millis();
    DisplayManager::setIP(getDisplayIP());
    DisplayManager::setTime(RTCManager::getTimeString());
    DisplayManager::setDate(RTCManager::getShortDateString());
  }
  DisplayManager::update();

  // Публикация полного состояния раз в 5 минут (300000 мс)
  static unsigned long lastMQTTFullPublish = 0;
  const unsigned long MQTT_FULL_PUBLISH_INTERVAL = 300000; // 5 минут
  if (millis() - lastMQTTFullPublish >= MQTT_FULL_PUBLISH_INTERVAL) {
    lastMQTTFullPublish = millis();
    if (MQTTManager::isConnected()) {
      MQTTManager::publishFullState();
    }
  }
}