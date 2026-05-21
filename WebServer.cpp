#include "WebServer.h"
#include "PumpControl.h"
#include "WiFiManager.h"
#include "ConfigManager.h"
#include "CalibrationManager.h"
#include "SchedulerManager.h"
#include "TaskRunner.h"
#include "TimeManager.h"
#include "LevelSensor.h"
#include "MQTTManager.h"
#include "SystemStatus.h"
#include "Settings.h"
#include "LedIndicator.h"
#include "StartGuard.h"
#include "version.h"
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <cstdio>

extern void broadcastStatus();

static ESP8266WebServer server(80);

static bool loadFromLittleFS(const String& path, const String& contentType) {
  if (!LittleFS.exists(path)) return false;
  File file = LittleFS.open(path, "r");
  if (!file) return false;
  
  server.sendHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate"));
  server.sendHeader(F("Pragma"), F("no-cache"));
  server.sendHeader(F("Expires"), F("0"));
  
  server.streamFile(file, contentType);
  file.close();
  return true;
}

void handleRoot() {
  if (isWiFiConnected() || isAPMode()) {
    if (!loadFromLittleFS("/index.html", "text/html")) {
      server.send(500, "text/plain", F("Failed to load index.html"));
    }
  } else {
    server.sendHeader(F("Location"), F("/wifi"));
    server.send(302, "text/plain", "");
  }
}

void handlePumpOn() {
  #ifdef DEBUG_ENABLED
  Serial.println(F("[WebServer] handlePumpOn() called"));
  #endif
  
  String error;
  if (!StartGuard::canStart(StartMode::PUMP_ONLY, error)) {
    server.send(409, "text/plain", error);
    return;
  }
  
  pumpOn();
  broadcastStatus();
  server.send(200, "text/plain", "ON");
}

void handlePumpOff() {
  #ifdef DEBUG_ENABLED
  Serial.println(F("[WebServer] handlePumpOff() called"));
  #endif
  
  if (SystemStatus::isCalibrating()) {
    server.send(409, "text/plain", F("Calibration in progress"));
    return;
  }
  
  if (TaskRunner::isRunning()) {
    TaskRunner::stop();
  }
  
  pumpOff();
  if (!TaskRunner::isRunning()) {
    broadcastStatus();
  }
  
  server.send(200, "text/plain", "OFF");
}

void handleApiStatus() {
  StaticJsonDocument<MAX_JSON_STATUS> doc;
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

  char response[MAX_JSON_STATUS];
  serializeJson(doc, response, sizeof(response));
  server.send(200, "application/json", response);
}

void handleDiagnosticsApi() {
  StaticJsonDocument<1024> doc;

  doc["uptime"] = millis() / 1000;
  doc["freeHeap"] = ESP.getFreeHeap();
  doc["sketchSize"] = ESP.getSketchSize();
  doc["sketchFree"] = ESP.getFreeSketchSpace();

  FSInfo fs_info;
  LittleFS.info(fs_info);
  JsonObject fs = doc.createNestedObject("littlefs");
  fs["total"] = fs_info.totalBytes;
  fs["used"] = fs_info.usedBytes;
  fs["free"] = fs_info.totalBytes - fs_info.usedBytes;

  JsonObject wifi = doc.createNestedObject("wifi");
  wifi["mode"] = WiFi.getMode();
  if (WiFi.status() == WL_CONNECTED) {
    wifi["ssid"] = WiFi.SSID();
    wifi["ip"] = WiFi.localIP().toString();
    wifi["rssi"] = WiFi.RSSI();
  } else {
    wifi["ssid"] = "";
    wifi["ip"] = "";
    wifi["rssi"] = 0;
  }

  doc["version"] = VERSION;

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleDiagnosticsPage() {
  if (!loadFromLittleFS("/diagnostics.html", "text/html")) {
    server.send(500, "text/plain", F("Failed to load diagnostics.html"));
  }
}

void handleWiFiConfig() {
  if (!loadFromLittleFS("/wifi.html", "text/html")) {
    server.send(500, "text/plain", F("Failed to load wifi.html"));
  }
}

void handleWiFiGet() {
  WiFiConfig* cfg = getWiFiConfig();
  char response[MAX_JSON_RESPONSE * 2];
  snprintf(response, sizeof(response),
           "{\"ssid\":\"%s\",\"password\":\"%s\",\"apPassword\":\"%s\",\"useStaticIP\":%s,\"staticIP\":\"%s\",\"gateway\":\"%s\",\"subnet\":\"%s\",\"dns1\":\"%s\",\"dns2\":\"%s\"}",
           cfg->ssid, cfg->password, cfg->apPassword,
           cfg->useStaticIP ? "true" : "false",
           cfg->staticIP, cfg->gateway, cfg->subnet,
           cfg->dns1, cfg->dns2);
  server.send(200, "application/json", response);
}

void handleSaveConfig() {
  if (!server.hasArg("ssid") || !server.hasArg("password")) {
    server.send(400, "text/plain", F("Missing SSID or Password"));
    return;
  }
  
  String ssid = server.arg("ssid");
  String password = server.arg("password");
  String apPassword = server.arg("apPassword");
  bool useStaticIP = server.hasArg("useStaticIP") && server.arg("useStaticIP") == "1";
  String staticIP = server.arg("staticIP");
  String gateway = server.arg("gateway");
  String subnet = server.arg("subnet");
  String dns1 = server.arg("dns1");
  String dns2 = server.arg("dns2");
  
  if (saveConfig(ssid.c_str(), password.c_str(), apPassword.c_str(), useStaticIP,
                 staticIP.c_str(), gateway.c_str(), subnet.c_str(),
                 dns1.c_str(), dns2.c_str())) {
    server.send(200, "text/html", 
      F("<h1>Configuration saved</h1><p>Device will restart in 3 seconds...</p>"
        "<script>setTimeout(function(){window.location.href='/'},3000);</script>"));
    delay(100);
    ESP.restart();
  } else {
    server.send(500, "text/html", F("<h1>Error saving configuration</h1>"));
  }
}

void handleResetConfig() {
  clearConfig();
  server.send(200, "text/html", 
    F("<h1>Configuration reset</h1><p>Device will restart in AP mode...</p>"
      "<script>setTimeout(function(){window.location.href='/'},3000);</script>"));
  delay(100);
  ESP.restart();
}

void handleScan() {
  ESP.wdtFeed();
  int n = WiFi.scanNetworks();
  ESP.wdtFeed();
  DynamicJsonDocument doc(MAX_JSON_RESPONSE);
  JsonArray arr = doc.to<JsonArray>();
  for (int i = 0; i < n; ++i) {
    JsonObject obj = arr.createNestedObject();
    obj["ssid"] = WiFi.SSID(i);
    obj["rssi"] = WiFi.RSSI(i);
  }
  String response;
  serializeJson(doc, response);
  WiFi.scanDelete();
  server.send(200, "application/json", response);
}

void handleCalibratePage() {
  if (!loadFromLittleFS("/calibrate.html", "text/html")) {
    server.send(500, "text/plain", F("Failed to load calibrate.html"));
  }
}

void handleCalibrateStart() {
  String error;
  if (!StartGuard::canStart(StartMode::CALIBRATE, error)) {
    server.send(409, "text/plain", error);
    return;
  }
  if (!server.hasArg("volume")) {
    server.send(400, "text/plain", F("Missing volume parameter"));
    return;
  }
  float volume = server.arg("volume").toFloat();
  if (volume <= 0) {
    server.send(400, "text/plain", F("Volume must be positive"));
    return;
  }
  
  CalibrationManager::setTargetVolume(volume);
  pumpOn();
  SystemStatus::setCalibrating(true);
  SystemStatus::setCalibrationStart(micros());
  SystemStatus::setCalibrationTarget(volume);
  SystemStatus::setFixedCalibration(false);
  MQTTManager::publishCalibrationStatus(true, volume, 0, CalibrationManager::isCalibrated());
  broadcastStatus();
  
  server.send(200, "text/plain", F("Calibration started"));
}

void handleCalibrateStop() {
  if (!SystemStatus::isCalibrating()) {
    server.send(409, "text/plain", F("No calibration in progress"));
    return;
  }

  if (SystemStatus::isFixedCalibration()) {
    pumpOff();
    SystemStatus::stopFixedCalibration();
    MQTTManager::publishCalibrationStatus(false, 0, 0, CalibrationManager::isCalibrated());
    broadcastStatus();
    server.send(200, "text/plain", F("Fixed calibration stopped"));
    return;
  }
  
  pumpOff();
  unsigned long elapsedMicros = micros() - SystemStatus::getCalibrationStart();
  float seconds = elapsedMicros / 1000000.0f;
  if (seconds <= 0) seconds = 0.001f;
  float flowRate = SystemStatus::getCalibrationTarget() / seconds;
  CalibrationManager::setFlowRate(flowRate);
  SystemStatus::resetCalibration();
  MQTTManager::publishCalibrationStatus(false, 0, 0, CalibrationManager::isCalibrated());
  broadcastStatus();
  
  char response[MAX_JSON_RESPONSE];
  snprintf(response, sizeof(response), 
           "{\"flowRate\":%.3f,\"timeSec\":%.3f}", flowRate, seconds);
  server.send(200, "application/json", response);
}

void handleCalibrateStatus() {
  if (SystemStatus::isCalibrating()) {
    unsigned long elapsedMicros = micros() - SystemStatus::getCalibrationStart();
    float seconds = elapsedMicros / 1000000.0f;
    char response[MAX_JSON_RESPONSE];
    snprintf(response, sizeof(response),
             "{\"calibrating\":true,\"timeSec\":%.1f,\"targetVolume\":%.0f,\"fixed\":%s,\"remainingSec\":%lu}",
             seconds, SystemStatus::getCalibrationTarget(),
             SystemStatus::isFixedCalibration() ? "true" : "false",
             SystemStatus::getFixedCalibrationRemaining());
    server.send(200, "application/json", response);
  } else {
    float flowRate = CalibrationManager::getFlowRate();
    char response[MAX_JSON_RESPONSE];
    snprintf(response, sizeof(response),
             "{\"calibrating\":false,\"flowRate\":%.3f}", flowRate);
    server.send(200, "application/json", response);
  }
}

void handleCalibrateFixedStart() {
    if (SystemStatus::isCalibrating()) {
        server.send(409, "text/plain", "Calibration already in progress");
        return;
    }
    if (!server.hasArg("time")) {
        server.send(400, "text/plain", "Missing time");
        return;
    }
    int time = server.arg("time").toInt();
    if (time <= 0) {
        server.send(400, "text/plain", "Invalid time");
        return;
    }
    
    String error;
    if (!StartGuard::canStart(StartMode::PUMP_ONLY, error)) {
        server.send(409, "text/plain", error);
        return;
    }
    
    pumpOn();
    SystemStatus::startFixedCalibration(time);
    MQTTManager::publishCalibrationStatus(true, 0, 0, CalibrationManager::isCalibrated());
    broadcastStatus();
    
    server.send(200, "text/plain", "OK");
}

void handleCalibrateFixedSave() {
    if (!server.hasArg("volume")) {
        server.send(400, "text/plain", "Missing volume");
        return;
    }
    float volume = server.arg("volume").toFloat();
    if (volume <= 0) {
        server.send(400, "text/plain", "Invalid volume");
        return;
    }

    unsigned long time = SystemStatus::getPendingFixedTime();
    if (time == 0) {
        server.send(409, "text/plain", "No pending fixed calibration");
        return;
    }

    pumpOff();
    float flowRate = volume / time;
    CalibrationManager::setFlowRate(flowRate);
    
    SystemStatus::clearPendingFixedTime();
    SystemStatus::resetCalibration();
    MQTTManager::publishCalibrationStatus(false, 0, 0, CalibrationManager::isCalibrated());
    broadcastStatus();
    
    server.send(200, "text/plain", "OK");
}

void handleCalibrateFixedCancel() {
    if (SystemStatus::getPendingFixedTime() == 0 && !SystemStatus::isFixedCalibration()) {
        server.send(409, "text/plain", "No fixed calibration in progress");
        return;
    }
    pumpOff();
    SystemStatus::cancelFixedCalibration();
    MQTTManager::publishCalibrationStatus(false, 0, 0, CalibrationManager::isCalibrated());
    broadcastStatus();
    server.send(200, "text/plain", "OK");
}

void handleCalibrateManual() {
    if (!server.hasArg("flowRate")) {
        server.send(400, "text/plain", "Missing flowRate");
        return;
    }
    float rate = server.arg("flowRate").toFloat();
    if (rate <= 0) {
        server.send(400, "text/plain", "Invalid flow rate");
        return;
    }
    CalibrationManager::setFlowRate(rate);
    server.send(200, "text/plain", "OK");
}

void handleCalibrateSet() {
    if (!server.hasArg("flowRate")) {
        server.send(400, "text/plain", "Missing flowRate");
        return;
    }
    float rate = server.arg("flowRate").toFloat();
    if (rate <= 0) {
        server.send(400, "text/plain", "Invalid flow rate");
        return;
    }
    CalibrationManager::setFlowRate(rate);
    server.send(200, "text/plain", "OK");
}

void handleSchedulePage() {
  if (!loadFromLittleFS("/schedule.html", "text/html")) {
    server.send(500, "text/plain", F("Failed to load schedule.html"));
  }
}

void handleScheduleList() {
  int count = SchedulerManager::getJobCount();
  DynamicJsonDocument doc(10000);
  JsonArray arr = doc.to<JsonArray>();
  for (int i = 0; i < count; i++) {
    const ScheduleJob& job = SchedulerManager::getJob(i);
    JsonObject obj = arr.createNestedObject();
    obj["id"] = job.id;
    obj["hour"] = job.hour;
    obj["minute"] = job.minute;
    obj["daysMask"] = job.daysMask;
    obj["volume"] = job.volume;
    obj["enabled"] = job.enabled;
  }
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleScheduleAdd() {
  if (!server.hasArg("hour") || !server.hasArg("minute") || !server.hasArg("daysMask") || !server.hasArg("volume")) {
    server.send(400, "text/plain", F("Missing parameters"));
    return;
  }
  ScheduleJob job;
  job.hour = server.arg("hour").toInt();
  job.minute = server.arg("minute").toInt();
  job.daysMask = server.arg("daysMask").toInt();
  job.volume = server.arg("volume").toInt();
  job.enabled = server.hasArg("enabled") ? server.arg("enabled").toInt() != 0 : true;
  
  if (job.hour > 23 || job.minute > 59 || job.volume <= 0) {
    server.send(400, "text/plain", F("Invalid values"));
    return;
  }

  if (job.daysMask == 0) {
    server.send(400, "text/plain", F("At least one day must be selected"));
    return;
  }

  if (!CalibrationManager::isCalibrated()) {
    server.send(409, "text/plain", F("Calibration required"));
    return;
  }

  if (SchedulerManager::getJobCount() >= MAX_SCHEDULE_JOBS) {
    server.send(409, "text/plain", F("Maximum jobs limit reached"));
    return;
  }

  if (!SchedulerManager::isTimeSlotAvailable(job)) {
    server.send(409, "text/plain", F("Time slot already occupied"));
    return;
  }
  
  if (SchedulerManager::addJob(job)) {
    MQTTManager::publishSchedule();
    server.send(200, "text/plain", "OK");
  } else {
    server.send(500, "text/plain", F("Failed to save"));
  }
}

void handleScheduleDelete() {
  if (!server.hasArg("id")) {
    server.send(400, "text/plain", F("Missing id"));
    return;
  }
  int id = server.arg("id").toInt();
  if (SchedulerManager::removeJob(id)) {
    MQTTManager::publishSchedule();
    server.send(200, "text/plain", "OK");
  } else {
    server.send(500, "text/plain", F("Failed to delete"));
  }
}

void handleScheduleRun() {
  String error;
  if (!StartGuard::canStart(StartMode::DISPENSE, error)) {
    server.send(409, "text/plain", error);
    return;
  }
  if (!server.hasArg("volume")) {
    server.send(400, "text/plain", F("Missing volume"));
    return;
  }
  int volume = server.arg("volume").toInt();
  if (volume <= 0) {
    server.send(400, "text/plain", F("Volume must be positive"));
    return;
  }
  
  float flowRate = CalibrationManager::getFlowRate();
  if (flowRate <= 0) {
    server.send(409, "text/plain", F("Flow rate not calibrated"));
    return;
  }
  
  unsigned long runTimeMs = (unsigned long)((volume / flowRate) * 1000);
  TaskRunner::start(runTimeMs, volume);
  broadcastStatus();
  
  server.send(200, "text/plain", "OK");
}

void handleScheduleToggle() {
  if (!server.hasArg("id") || !server.hasArg("enabled")) {
    server.send(400, "text/plain", "Missing id or enabled");
    return;
  }
  int id = server.arg("id").toInt();
  bool enabled = server.arg("enabled").toInt() != 0;
  
  for (int i = 0; i < SchedulerManager::getJobCount(); i++) {
    ScheduleJob job = SchedulerManager::getJob(i);
    if (job.id == id) {
      job.enabled = enabled;
      if (SchedulerManager::updateJob(job)) {
        MQTTManager::publishSchedule();
        server.send(200, "text/plain", "OK");
      } else {
        server.send(500, "text/plain", "Failed to update job");
      }
      return;
    }
  }
  server.send(404, "text/plain", "Job not found");
}

void handleTimePage() {
  if (!loadFromLittleFS("/time.html", "text/html")) {
    server.send(500, "text/plain", F("Failed to load time.html"));
  }
}

void handleTimeGet() {
  char response[MAX_JSON_RESPONSE];
  snprintf(response, sizeof(response),
           "{\"time\":\"%s\",\"date\":\"%s\"}",
           TimeManager::getTimeString().c_str(),
           TimeManager::getDateString().c_str());
  server.send(200, "application/json", response);
}

void handleTimeSet() {
  if (!server.hasArg("year") || !server.hasArg("month") || !server.hasArg("day") ||
      !server.hasArg("hour") || !server.hasArg("minute") || !server.hasArg("second")) {
    server.send(400, "text/plain", F("Missing parameters"));
    return;
  }
  
  uint16_t year = server.arg("year").toInt();
  uint8_t month = server.arg("month").toInt();
  uint8_t day = server.arg("day").toInt();
  uint8_t hour = server.arg("hour").toInt();
  uint8_t minute = server.arg("minute").toInt();
  uint8_t second = server.arg("second").toInt();
  
  if (TimeManager::setDateTime(year, month, day, hour, minute, second)) {
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", F("Invalid date/time"));
  }
}

void handleTimeNTP() {
  if (WiFi.status() != WL_CONNECTED) {
    server.send(400, "text/plain", F("WiFi not connected"));
    return;
  }
  
  if (TimeManager::syncNTP()) {
    server.send(200, "text/plain", "OK");
  } else {
    server.send(500, "text/plain", F("NTP sync failed"));
  }
}

void handleMQTTConfigPage() {
  if (!loadFromLittleFS("/mqtt.html", "text/html")) {
    server.send(500, "text/plain", F("Failed to load mqtt.html"));
  }
}

void handleMQTTGet() {
  MQTTConfig* cfg = getMQTTConfig();
  char response[MAX_JSON_RESPONSE * 2];
  snprintf(response, sizeof(response),
           "{\"server\":\"%s\",\"port\":%d,\"user\":\"%s\",\"password\":\"%s\",\"prefix\":\"%s\",\"ntpServer\":\"%s\",\"timezone\":%d}",
           cfg->server, cfg->port, cfg->user, cfg->password,
           cfg->prefix, cfg->ntpServer, cfg->timezone);
  server.send(200, "application/json", response);
}

void handleMQTTSave() {
  if (!server.hasArg("server") || !server.hasArg("port") || !server.hasArg("prefix")) {
    server.send(400, "text/plain", F("Missing parameters"));
    return;
  }
  MQTTConfig cfg;
  strlcpy(cfg.server, server.arg("server").c_str(), sizeof(cfg.server));
  cfg.port = server.arg("port").toInt();
  strlcpy(cfg.user, server.arg("user").c_str(), sizeof(cfg.user));
  strlcpy(cfg.password, server.arg("password").c_str(), sizeof(cfg.password));
  strlcpy(cfg.prefix, server.arg("prefix").c_str(), sizeof(cfg.prefix));
  if (strlen(cfg.prefix) == 0) strlcpy(cfg.prefix, MQTT_DEFAULT_PREFIX, sizeof(cfg.prefix));
  strlcpy(cfg.ntpServer, server.hasArg("ntpServer") ? server.arg("ntpServer").c_str() : "pool.ntp.org", sizeof(cfg.ntpServer));
  cfg.timezone = server.hasArg("timezone") ? server.arg("timezone").toInt() : MQTT_DEFAULT_TIMEZONE;
  
  if (saveMQTTConfig(&cfg)) {
    server.send(200, "text/html",
      F("<h1>MQTT configuration saved</h1><p>Device will restart in 3 seconds...</p>"
        "<script>setTimeout(function(){window.location.href='/'},3000);</script>"));
    delay(100);
    ESP.restart();
  } else {
    server.send(500, "text/plain", F("Failed to save"));
  }
}

void handleMQTTStatus() {
  char response[MAX_JSON_RESPONSE];
  snprintf(response, sizeof(response),
           "{\"status\":\"%s\"}", MQTTManager::isConnected() ? "connected" : "disconnected");
  server.send(200, "application/json", response);
}

void handleMQTTDiscovery() {
  if (!MQTTManager::isConnected()) {
    server.send(400, "text/plain", F("MQTT not connected"));
    return;
  }
  MQTTManager::publishDiscovery();
  server.send(200, "text/plain", F("Discovery sent"));
}

void handleCalibrationStatus() {
  char response[MAX_JSON_RESPONSE];
  snprintf(response, sizeof(response),
           "{\"calibrated\":%s}", CalibrationManager::isCalibrated() ? "true" : "false");
  server.send(200, "application/json", response);
}

void handleGetBrightness() {
  DeviceConfig cfg;
  loadDeviceConfig(cfg);
  char response[64];
  snprintf(response, sizeof(response), "{\"brightness\":%d}", cfg.ledBrightness);
  server.send(200, "application/json", response);
}

void handleSetBrightness() {
  if (!server.hasArg("value")) {
    server.send(400, "text/plain", "Missing value");
    return;
  }
  int percent = server.arg("value").toInt();
  if (percent < 0) percent = 0;
  if (percent > 100) percent = 100;
  
  uint8_t val = (uint8_t)(percent * 255 / 100);
  
  DeviceConfig cfg;
  loadDeviceConfig(cfg);
  cfg.ledBrightness = val;
  if (saveDeviceConfig(cfg)) {
    LedIndicator::setBrightness(val);
    MQTTManager::publishBrightness();
    server.send(200, "text/plain", "OK");
  } else {
    server.send(500, "text/plain", "Failed to save");
  }
}

void handleLevelConfigGet() {
  DeviceConfig cfg;
  loadDeviceConfig(cfg);
  char response[64];
  snprintf(response, sizeof(response), "{\"activeLevel\":%d}", static_cast<uint8_t>(cfg.levelActiveLevel));
  server.send(200, "application/json", response);
}

void handleLevelConfigSet() {
  if (!server.hasArg("activeLevel")) {
    server.send(400, "text/plain", "Missing activeLevel");
    return;
  }
  int level = server.arg("activeLevel").toInt();
  if (level != 0 && level != 1) {
    server.send(400, "text/plain", "activeLevel must be 0 (LOW) or 1 (HIGH)");
    return;
  }
  
  DeviceConfig cfg;
  loadDeviceConfig(cfg);
  cfg.levelActiveLevel = static_cast<LevelActive>(level);
  if (saveDeviceConfig(cfg)) {
    LevelSensor::begin(LEVEL_PIN, cfg.levelActiveLevel);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(500, "text/plain", "Failed to save");
  }
}

void handleConfigPage() {
  if (!loadFromLittleFS("/config.html", "text/html")) {
    server.send(500, "text/plain", "Failed to load config.html");
  }
}

void handleExportConfig() {
  DynamicJsonDocument* doc = new DynamicJsonDocument(8192);
  if (!doc) {
    server.send(500, "text/plain", "Out of memory");
    return;
  }
  
  JsonObject wifiObj = doc->createNestedObject("wifi");
  WiFiConfig* wifi = getWiFiConfig();
  wifiObj["ssid"] = wifi->ssid;
  wifiObj["password"] = wifi->password;
  wifiObj["apPassword"] = wifi->apPassword;
  wifiObj["useStaticIP"] = wifi->useStaticIP;
  wifiObj["staticIP"] = wifi->staticIP;
  wifiObj["gateway"] = wifi->gateway;
  wifiObj["subnet"] = wifi->subnet;
  wifiObj["dns1"] = wifi->dns1;
  wifiObj["dns2"] = wifi->dns2;

  JsonObject mqttObj = doc->createNestedObject("mqtt");
  MQTTConfig* mqtt = getMQTTConfig();
  mqttObj["server"] = mqtt->server;
  mqttObj["port"] = mqtt->port;
  mqttObj["user"] = mqtt->user;
  mqttObj["password"] = mqtt->password;
  mqttObj["prefix"] = mqtt->prefix;
  mqttObj["ntpServer"] = mqtt->ntpServer;
  mqttObj["timezone"] = mqtt->timezone;

  JsonObject calibObj = doc->createNestedObject("calibration");
  calibObj["flowRate"] = CalibrationManager::getFlowRate();
  calibObj["targetVolume"] = CalibrationManager::getTargetVolume();

  JsonObject deviceObj = doc->createNestedObject("device");
  DeviceConfig devCfg;
  loadDeviceConfig(devCfg);
  deviceObj["ledBrightness"] = devCfg.ledBrightness;
  deviceObj["levelActiveLevel"] = static_cast<uint8_t>(devCfg.levelActiveLevel);

  JsonArray scheduleArray = doc->createNestedArray("schedule");
  for (int i = 0; i < SchedulerManager::getJobCount(); i++) {
    const ScheduleJob& job = SchedulerManager::getJob(i);
    JsonObject jobObj = scheduleArray.createNestedObject();
    jobObj["id"] = job.id;
    jobObj["hour"] = job.hour;
    jobObj["minute"] = job.minute;
    jobObj["daysMask"] = job.daysMask;
    jobObj["volume"] = job.volume;
    jobObj["enabled"] = job.enabled;
  }

  String response;
  serializeJson(*doc, response);
  delete doc;
  server.sendHeader("Content-Disposition", "attachment; filename=afd_config.json");
  server.send(200, "application/json", response);
}

void handleImportConfig() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Missing JSON data");
    return;
  }
  String body = server.arg("plain");
  DynamicJsonDocument* doc = new DynamicJsonDocument(8192);
  if (!doc) {
    server.send(500, "text/plain", "Out of memory");
    return;
  }
  DeserializationError error = deserializeJson(*doc, body);
  if (error) {
    delete doc;
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }

  if (doc->containsKey("wifi")) {
    JsonObject wifiObj = (*doc)["wifi"];
    saveConfig(
      wifiObj["ssid"] | "",
      wifiObj["password"] | "",
      wifiObj["apPassword"] | "",
      wifiObj["useStaticIP"] | false,
      wifiObj["staticIP"] | "",
      wifiObj["gateway"] | "",
      wifiObj["subnet"] | "",
      wifiObj["dns1"] | "",
      wifiObj["dns2"] | ""
    );
  }

  if (doc->containsKey("mqtt")) {
    JsonObject mqttObj = (*doc)["mqtt"];
    MQTTConfig mqttCfg;
    strlcpy(mqttCfg.server, mqttObj["server"] | "", sizeof(mqttCfg.server));
    mqttCfg.port = mqttObj["port"] | MQTT_DEFAULT_PORT;
    strlcpy(mqttCfg.user, mqttObj["user"] | "", sizeof(mqttCfg.user));
    strlcpy(mqttCfg.password, mqttObj["password"] | "", sizeof(mqttCfg.password));
    strlcpy(mqttCfg.prefix, mqttObj["prefix"] | MQTT_DEFAULT_PREFIX, sizeof(mqttCfg.prefix));
    strlcpy(mqttCfg.ntpServer, mqttObj["ntpServer"] | "pool.ntp.org", sizeof(mqttCfg.ntpServer));
    mqttCfg.timezone = mqttObj["timezone"] | MQTT_DEFAULT_TIMEZONE;
    saveMQTTConfig(&mqttCfg);
  }

  if (doc->containsKey("calibration")) {
    JsonObject calibObj = (*doc)["calibration"];
    CalibrationManager::setFlowRate(calibObj["flowRate"] | 0.0f);
    CalibrationManager::setTargetVolume(calibObj["targetVolume"] | 100.0f);
  }

  if (doc->containsKey("device")) {
    JsonObject devObj = (*doc)["device"];
    DeviceConfig devCfg;
    devCfg.ledBrightness = devObj["ledBrightness"] | DEFAULT_LED_BRIGHTNESS;
    uint8_t level = devObj["levelActiveLevel"] | static_cast<uint8_t>(LevelActive::ACTIVE_HIGH);
    devCfg.levelActiveLevel = static_cast<LevelActive>(level);
    saveDeviceConfig(devCfg);
    LedIndicator::setBrightness(devCfg.ledBrightness);
    LevelSensor::begin(LEVEL_PIN, devCfg.levelActiveLevel);
  }

  if (doc->containsKey("schedule")) {
    JsonArray scheduleArray = (*doc)["schedule"];
    
    // Проверка свободного места перед записью
    size_t jsonSize = measureJson(scheduleArray);
    if (!hasEnoughSpace(jsonSize)) {
      delete doc;
      server.send(500, "text/plain", "Insufficient space to save schedule");
      return;
    }
    
    File file = LittleFS.open("/schedule.json", "w");
    if (file) {
      serializeJson(scheduleArray, file);
      file.close();
    } else {
      delete doc;
      server.send(500, "text/plain", "Failed to open schedule file");
      return;
    }
    SchedulerManager::load();
  }

  delete doc;
  server.send(200, "text/plain", "OK");
  delay(1000);
  ESP.restart();
}

void handleNotFound() {
  String path = server.uri();
  String contentType = "text/plain";
  if (path.endsWith(".html")) contentType = "text/html";
  else if (path.endsWith(".css")) contentType = "text/css";
  else if (path.endsWith(".js")) contentType = "application/javascript";
  else if (path.endsWith(".png")) contentType = "image/png";
  else if (path.endsWith(".jpg")) contentType = "image/jpeg";
  else if (path.endsWith(".ico")) contentType = "image/x-icon";
  else if (path.endsWith(".svg")) contentType = "image/svg+xml";
  else if (path.endsWith(".json")) contentType = "application/json";
  
  if (loadFromLittleFS(path, contentType)) {
    return;
  }
  server.send(404, "text/plain", "Not found");
}

void startWebServer() {
  server.on("/", handleRoot);
  server.on("/pump/on", handlePumpOn);
  server.on("/pump/off", handlePumpOff);
  server.on("/api/status", handleApiStatus);
  server.on("/api/diagnostics", handleDiagnosticsApi);
  server.on("/diagnostics", handleDiagnosticsPage);
  server.on("/wifi", handleWiFiConfig);
  server.on("/wifi/get", handleWiFiGet);
  server.on("/saveconfig", handleSaveConfig);
  server.on("/resetconfig", handleResetConfig);
  server.on("/scan", handleScan);
  server.on("/calibrate", handleCalibratePage);
  server.on("/calibrate/start", handleCalibrateStart);
  server.on("/calibrate/stop", handleCalibrateStop);
  server.on("/calibrate/status", handleCalibrateStatus);
  server.on("/calibrate/fixed/start", handleCalibrateFixedStart);
  server.on("/calibrate/fixed/save", handleCalibrateFixedSave);
  server.on("/calibrate/fixed/cancel", handleCalibrateFixedCancel);
  server.on("/calibrate/manual", handleCalibrateManual);
  server.on("/calibrate/set", handleCalibrateSet);
  server.on("/schedule", handleSchedulePage);
  server.on("/schedule/list", handleScheduleList);
  server.on("/schedule/add", handleScheduleAdd);
  server.on("/schedule/delete", handleScheduleDelete);
  server.on("/schedule/run", handleScheduleRun);
  server.on("/schedule/toggle", handleScheduleToggle);
  server.on("/time", handleTimePage);
  server.on("/time/get", handleTimeGet);
  server.on("/time/set", handleTimeSet);
  server.on("/time/ntp", handleTimeNTP);
  server.on("/mqtt", handleMQTTConfigPage);
  server.on("/mqtt/get", handleMQTTGet);
  server.on("/mqtt/save", handleMQTTSave);
  server.on("/mqtt/status", handleMQTTStatus);
  server.on("/mqtt/discovery", handleMQTTDiscovery);
  server.on("/calibration/status", handleCalibrationStatus);
  server.on("/api/brightness", HTTP_GET, handleGetBrightness);
  server.on("/api/brightness", HTTP_POST, handleSetBrightness);
  server.on("/api/level/config", HTTP_GET, handleLevelConfigGet);
  server.on("/api/level/config", HTTP_POST, handleLevelConfigSet);
  server.on("/config", handleConfigPage);
  server.on("/api/export", handleExportConfig);
  server.on("/api/import", HTTP_POST, handleImportConfig);
  
  server.onNotFound(handleNotFound);
  
  server.begin();
  #ifdef DEBUG_ENABLED
  Serial.println(F("Web server started"));
  #endif
}

void handleWebServer() {
  server.handleClient();
}