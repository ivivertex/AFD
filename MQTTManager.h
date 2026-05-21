#ifndef MQTTMANAGER_H
#define MQTTMANAGER_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "ConfigManager.h"
#include "Settings.h"

class MQTTManager {
public:
  static void begin(MQTTConfig* cfg);
  static void loop();
  static void publishState(bool pumpOn);
  static void publishLevel(bool levelOk);
  static void publishAvailability(bool online);
  static void publishCalibrationStatus(bool calibrating, float target, float elapsed, bool calibrated);
  static bool isConnected();
  static void publishDiscovery();
  static void publishDispenseFinished(bool success, int volume, unsigned long elapsedMs);
  static void publishBrightness();
  static void publishSchedule();
  static void publishFullState();   // Новая функция для периодической публикации всех состояний

private:
  static void reconnect();
  static void callback(char* topic, byte* payload, unsigned int length);
  static void sendDiscovery();
  static void publishScheduleResult(bool success, const char* message);

  static WiFiClient wifiClient;
  static PubSubClient client;
  static MQTTConfig* config;
  static bool initialized;
  static unsigned long lastReconnectAttempt;
  static const unsigned long RECONNECT_INTERVAL;
  
  static float dispenseVolume;
};

#endif