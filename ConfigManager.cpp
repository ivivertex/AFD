#include "ConfigManager.h"
#include <ArduinoJson.h>

static WiFiConfig wifiConfig;
static MQTTConfig mqttConfig;
static const char* wifiFile = "/wifi_config.json";
static const char* mqttFile = "/mqtt_config.json";
static const char* deviceFile = "/device.json";

#define LITTLEFS_MOUNT_RETRIES 3
#define LITTLEFS_MOUNT_DELAY_MS 500

bool initConfig() {
  for (int i = 0; i < LITTLEFS_MOUNT_RETRIES; i++) {
    if (LittleFS.begin()) {
      #ifdef DEBUG_ENABLED
      Serial.println(F("LittleFS mounted successfully"));
      #endif
      return true;
    }
    #ifdef DEBUG_ENABLED
    Serial.printf_P(PSTR("LittleFS mount attempt %d failed, retrying...\n"), i+1);
    #endif
    delay(LITTLEFS_MOUNT_DELAY_MS);
  }
  #ifdef DEBUG_ENABLED
  Serial.println(F("Failed to mount LittleFS after all attempts"));
  #endif
  return false;
}

bool hasEnoughSpace(size_t requiredBytes) {
  FSInfo fs_info;
  if (!LittleFS.info(fs_info)) {
    #ifdef DEBUG_ENABLED
    Serial.println(F("Failed to get LittleFS info"));
    #endif
    return false;
  }
  size_t freeBytes = fs_info.totalBytes - fs_info.usedBytes;
  // Добавляем 1 КБ запаса
  if (freeBytes >= requiredBytes + 1024) {
    return true;
  }
  #ifdef DEBUG_ENABLED
  Serial.printf_P(PSTR("Insufficient space: required %u, free %u\n"), requiredBytes + 1024, freeBytes);
  #endif
  return false;
}

bool loadConfig() {
  if (!LittleFS.exists(wifiFile)) {
    wifiConfig.ssid[0] = 0;
    wifiConfig.password[0] = 0;
    wifiConfig.apPassword[0] = 0;
    wifiConfig.useStaticIP = false;
    wifiConfig.staticIP[0] = 0;
    wifiConfig.gateway[0] = 0;
    wifiConfig.subnet[0] = 0;
    wifiConfig.dns1[0] = 0;
    wifiConfig.dns2[0] = 0;
    return false;
  }
  File file = LittleFS.open(wifiFile, "r");
  if (!file) return false;
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  if (error) return false;
  
  strlcpy(wifiConfig.ssid, doc["ssid"] | "", MAX_SSID_LEN);
  strlcpy(wifiConfig.password, doc["password"] | "", MAX_PASS_LEN);
  strlcpy(wifiConfig.apPassword, doc["apPassword"] | "", MAX_PASS_LEN);
  wifiConfig.useStaticIP = doc["useStaticIP"] | false;
  strlcpy(wifiConfig.staticIP, doc["staticIP"] | "", MAX_IP_STR_LEN);
  strlcpy(wifiConfig.gateway, doc["gateway"] | "", MAX_IP_STR_LEN);
  strlcpy(wifiConfig.subnet, doc["subnet"] | "", MAX_IP_STR_LEN);
  strlcpy(wifiConfig.dns1, doc["dns1"] | "", MAX_IP_STR_LEN);
  strlcpy(wifiConfig.dns2, doc["dns2"] | "", MAX_IP_STR_LEN);
  return true;
}

bool saveConfig(const char* ssid, const char* password, const char* apPassword,
                bool useStaticIP, const char* staticIP, const char* gateway, const char* subnet,
                const char* dns1, const char* dns2) {
  // Предварительная оценка размера JSON
  StaticJsonDocument<1024> doc;
  doc["ssid"] = ssid;
  doc["password"] = password;
  doc["apPassword"] = apPassword;
  doc["useStaticIP"] = useStaticIP;
  doc["staticIP"] = staticIP;
  doc["gateway"] = gateway;
  doc["subnet"] = subnet;
  doc["dns1"] = dns1;
  doc["dns2"] = dns2;
  size_t jsonSize = measureJson(doc);
  if (!hasEnoughSpace(jsonSize)) {
    return false;
  }
  
  File file = LittleFS.open(wifiFile, "w");
  if (!file) return false;
  if (serializeJson(doc, file) == 0) {
    file.close();
    return false;
  }
  file.close();
  strlcpy(wifiConfig.ssid, ssid, MAX_SSID_LEN);
  strlcpy(wifiConfig.password, password, MAX_PASS_LEN);
  strlcpy(wifiConfig.apPassword, apPassword, MAX_PASS_LEN);
  wifiConfig.useStaticIP = useStaticIP;
  strlcpy(wifiConfig.staticIP, staticIP, MAX_IP_STR_LEN);
  strlcpy(wifiConfig.gateway, gateway, MAX_IP_STR_LEN);
  strlcpy(wifiConfig.subnet, subnet, MAX_IP_STR_LEN);
  strlcpy(wifiConfig.dns1, dns1, MAX_IP_STR_LEN);
  strlcpy(wifiConfig.dns2, dns2, MAX_IP_STR_LEN);
  return true;
}

bool isWiFiConfigValid() {
  return strlen(wifiConfig.ssid) > 0;
}

WiFiConfig* getWiFiConfig() {
  return &wifiConfig;
}

bool loadMQTTConfig() {
  if (!LittleFS.exists(mqttFile)) {
    mqttConfig.server[0] = 0;
    mqttConfig.port = MQTT_DEFAULT_PORT;
    mqttConfig.user[0] = 0;
    mqttConfig.password[0] = 0;
    strlcpy(mqttConfig.prefix, MQTT_DEFAULT_PREFIX, sizeof(mqttConfig.prefix));
    strlcpy(mqttConfig.ntpServer, "pool.ntp.org", sizeof(mqttConfig.ntpServer));
    mqttConfig.timezone = MQTT_DEFAULT_TIMEZONE;
    return false;
  }
  File file = LittleFS.open(mqttFile, "r");
  if (!file) return false;
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  if (error) return false;
  
  strlcpy(mqttConfig.server, doc["server"] | "", MAX_IP_STR_LEN);
  mqttConfig.port = doc["port"] | MQTT_DEFAULT_PORT;
  strlcpy(mqttConfig.user, doc["user"] | "", sizeof(mqttConfig.user));
  strlcpy(mqttConfig.password, doc["password"] | "", sizeof(mqttConfig.password));
  strlcpy(mqttConfig.prefix, doc["prefix"] | MQTT_DEFAULT_PREFIX, sizeof(mqttConfig.prefix));
  strlcpy(mqttConfig.ntpServer, doc["ntpServer"] | "pool.ntp.org", sizeof(mqttConfig.ntpServer));
  mqttConfig.timezone = doc["timezone"] | MQTT_DEFAULT_TIMEZONE;
  return true;
}

bool saveMQTTConfig(const MQTTConfig* cfg) {
  StaticJsonDocument<1024> doc;
  doc["server"] = cfg->server;
  doc["port"] = cfg->port;
  doc["user"] = cfg->user;
  doc["password"] = cfg->password;
  doc["prefix"] = cfg->prefix;
  doc["ntpServer"] = cfg->ntpServer;
  doc["timezone"] = cfg->timezone;
  size_t jsonSize = measureJson(doc);
  if (!hasEnoughSpace(jsonSize)) {
    return false;
  }
  
  File file = LittleFS.open(mqttFile, "w");
  if (!file) return false;
  if (serializeJson(doc, file) == 0) {
    file.close();
    return false;
  }
  file.close();
  mqttConfig = *cfg;
  return true;
}

MQTTConfig* getMQTTConfig() {
  return &mqttConfig;
}

bool loadDeviceConfig(DeviceConfig& cfg) {
  if (!LittleFS.exists(deviceFile)) {
    cfg.ledBrightness = DEFAULT_LED_BRIGHTNESS;
    cfg.levelActiveLevel = LevelActive::ACTIVE_HIGH;
    return false;
  }
  File file = LittleFS.open(deviceFile, "r");
  if (!file) return false;
  StaticJsonDocument<128> doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  if (error) {
    cfg.ledBrightness = DEFAULT_LED_BRIGHTNESS;
    cfg.levelActiveLevel = LevelActive::ACTIVE_HIGH;
    return false;
  }
  cfg.ledBrightness = doc["ledBrightness"] | DEFAULT_LED_BRIGHTNESS;
  uint8_t level = doc["levelActiveLevel"] | static_cast<uint8_t>(LevelActive::ACTIVE_HIGH);
  cfg.levelActiveLevel = static_cast<LevelActive>(level);
  return true;
}

bool saveDeviceConfig(const DeviceConfig& cfg) {
  StaticJsonDocument<128> doc;
  doc["ledBrightness"] = cfg.ledBrightness;
  doc["levelActiveLevel"] = static_cast<uint8_t>(cfg.levelActiveLevel);
  size_t jsonSize = measureJson(doc);
  if (!hasEnoughSpace(jsonSize)) {
    return false;
  }
  
  File file = LittleFS.open(deviceFile, "w");
  if (!file) return false;
  if (serializeJson(doc, file) == 0) {
    file.close();
    return false;
  }
  file.close();
  return true;
}

bool clearConfig() {
  bool success = true;
  
  if (LittleFS.exists(wifiFile)) {
    if (!LittleFS.remove(wifiFile)) success = false;
  }
  if (LittleFS.exists(mqttFile)) {
    if (!LittleFS.remove(mqttFile)) success = false;
  }
  if (LittleFS.exists("/calibration.json")) {
    if (!LittleFS.remove("/calibration.json")) success = false;
  }
  if (LittleFS.exists(deviceFile)) {
    if (!LittleFS.remove(deviceFile)) success = false;
  }
  if (LittleFS.exists("/schedule.json")) {
    if (!LittleFS.remove("/schedule.json")) success = false;
  }
  
  wifiConfig.ssid[0] = 0;
  wifiConfig.password[0] = 0;
  wifiConfig.apPassword[0] = 0;
  wifiConfig.useStaticIP = false;
  wifiConfig.staticIP[0] = 0;
  wifiConfig.gateway[0] = 0;
  wifiConfig.subnet[0] = 0;
  wifiConfig.dns1[0] = 0;
  wifiConfig.dns2[0] = 0;
  
  mqttConfig.server[0] = 0;
  mqttConfig.port = MQTT_DEFAULT_PORT;
  mqttConfig.user[0] = 0;
  mqttConfig.password[0] = 0;
  strlcpy(mqttConfig.prefix, MQTT_DEFAULT_PREFIX, sizeof(mqttConfig.prefix));
  strlcpy(mqttConfig.ntpServer, "pool.ntp.org", sizeof(mqttConfig.ntpServer));
  mqttConfig.timezone = MQTT_DEFAULT_TIMEZONE;
  
  return success;
}