#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <Arduino.h>
#include <LittleFS.h>
#include "Settings.h"

struct WiFiConfig {
  char ssid[MAX_SSID_LEN];
  char password[MAX_PASS_LEN];
  char apPassword[MAX_PASS_LEN];
  bool useStaticIP;
  char staticIP[MAX_IP_STR_LEN];
  char gateway[MAX_IP_STR_LEN];
  char subnet[MAX_IP_STR_LEN];
  char dns1[MAX_IP_STR_LEN];
  char dns2[MAX_IP_STR_LEN];
};

struct MQTTConfig {
  char server[MAX_IP_STR_LEN];
  uint16_t port;
  char user[32];
  char password[32];
  char prefix[16];
  char ntpServer[64];
  int8_t timezone;
};

struct DeviceConfig {
  uint8_t ledBrightness;
  LevelActive levelActiveLevel;
};

bool initConfig();
bool loadConfig();
bool saveConfig(const char* ssid, const char* password, const char* apPassword,
                bool useStaticIP, const char* staticIP, const char* gateway, const char* subnet,
                const char* dns1, const char* dns2);
bool isWiFiConfigValid();
WiFiConfig* getWiFiConfig();

bool loadMQTTConfig();
bool saveMQTTConfig(const MQTTConfig* cfg);
MQTTConfig* getMQTTConfig();

bool loadDeviceConfig(DeviceConfig& cfg);
bool saveDeviceConfig(const DeviceConfig& cfg);

bool clearConfig();

// Новая функция проверки свободного места
bool hasEnoughSpace(size_t requiredBytes);

#endif