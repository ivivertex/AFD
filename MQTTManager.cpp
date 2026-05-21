#include "MQTTManager.h"
#include "PumpControl.h"
#include "LevelSensor.h"
#include "SystemStatus.h"
#include "CalibrationManager.h"
#include "TaskRunner.h"
#include "SchedulerManager.h"
#include "Settings.h"
#include "ConfigManager.h"
#include "LedIndicator.h"
#include "StartGuard.h"
#include "version.h"
#include <ArduinoJson.h>
#include <cstdio>

WiFiClient MQTTManager::wifiClient;
PubSubClient MQTTManager::client(wifiClient);
MQTTConfig* MQTTManager::config = nullptr;
bool MQTTManager::initialized = false;
unsigned long MQTTManager::lastReconnectAttempt = 0;
const unsigned long MQTTManager::RECONNECT_INTERVAL = 5000;
float MQTTManager::dispenseVolume = 100.0f;

static String getDeviceId() {
  return "esp_pump_" + String(ESP.getChipId(), HEX);
}

void MQTTManager::begin(MQTTConfig* cfg) {
  config = cfg;
  if (strlen(config->server) == 0) {
    #ifdef DEBUG_ENABLED
    Serial.println(F("MQTT: no server configured"));
    #endif
    return;
  }
  client.setServer(config->server, config->port);
  client.setCallback(callback);
  client.setBufferSize(MAX_JSON_LARGE);
  initialized = true;
  lastReconnectAttempt = millis() - RECONNECT_INTERVAL;
}

void MQTTManager::loop() {
  if (!initialized || strlen(config->server) == 0) return;
  if (!client.connected()) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt >= RECONNECT_INTERVAL) {
      lastReconnectAttempt = now;
      reconnect();
    }
  } else {
    client.loop();
  }
}

void MQTTManager::reconnect() {
  if (!initialized) return;
  
  String clientId = "ESP_Pump_" + String(ESP.getChipId(), HEX);
  bool connected = false;
  
  #ifdef DEBUG_ENABLED
  Serial.print(F("MQTT connecting to "));
  Serial.print(config->server);
  Serial.print(F(":"));
  Serial.println(config->port);
  #endif
  
  wifiClient.setTimeout(2000);
  
  if (strlen(config->user) > 0) {
    connected = client.connect(clientId.c_str(), config->user, config->password);
  } else {
    connected = client.connect(clientId.c_str());
  }
  
  if (connected) {
    #ifdef DEBUG_ENABLED
    Serial.println(F("MQTT connected"));
    #endif
    
    char topicBuf[64];
    // Подписки с QOS 1 для надёжности команд
    snprintf(topicBuf, sizeof(topicBuf), "%s/set", config->prefix);
    client.subscribe(topicBuf, 1);
    snprintf(topicBuf, sizeof(topicBuf), "%s/calibrate/volume/set", config->prefix);
    client.subscribe(topicBuf, 1);
    snprintf(topicBuf, sizeof(topicBuf), "%s/calibrate/start", config->prefix);
    client.subscribe(topicBuf, 1);
    snprintf(topicBuf, sizeof(topicBuf), "%s/calibrate/stop", config->prefix);
    client.subscribe(topicBuf, 1);
    snprintf(topicBuf, sizeof(topicBuf), "%s/dispense/volume/set", config->prefix);
    client.subscribe(topicBuf, 1);
    snprintf(topicBuf, sizeof(topicBuf), "%s/dispense/run", config->prefix);
    client.subscribe(topicBuf, 1);
    snprintf(topicBuf, sizeof(topicBuf), "%s/dispense/stop", config->prefix);
    client.subscribe(topicBuf, 1);
    
    snprintf(topicBuf, sizeof(topicBuf), "%s/schedule/get", config->prefix);
    client.subscribe(topicBuf, 1);
    snprintf(topicBuf, sizeof(topicBuf), "%s/schedule/add", config->prefix);
    client.subscribe(topicBuf, 1);
    snprintf(topicBuf, sizeof(topicBuf), "%s/schedule/remove", config->prefix);
    client.subscribe(topicBuf, 1);
    snprintf(topicBuf, sizeof(topicBuf), "%s/schedule/update", config->prefix);
    client.subscribe(topicBuf, 1);
    
    snprintf(topicBuf, sizeof(topicBuf), "%s/brightness/set", config->prefix);
    client.subscribe(topicBuf, 1);
    
    delay(500);
    sendDiscovery();
    
    // Публикации с retain
    publishAvailability(true);
    delay(100);
    publishState(isPumpOn());
    delay(100);
    publishLevel(LevelSensor::isLevelOk());
    delay(100);
    publishCalibrationStatus(SystemStatus::isCalibrating(), 
                             CalibrationManager::getTargetVolume(), 
                             0,
                             CalibrationManager::isCalibrated());
    
    snprintf(topicBuf, sizeof(topicBuf), "%s/dispense/volume", config->prefix);
    client.publish(topicBuf, String(dispenseVolume).c_str(), true);
    
    publishBrightness();
    
    snprintf(topicBuf, sizeof(topicBuf), "%s/calibrate/volume", config->prefix);
    client.publish(topicBuf, String(CalibrationManager::getTargetVolume()).c_str(), true);
    
    publishSchedule();
  } else {
    #ifdef DEBUG_ENABLED
    Serial.print(F("MQTT connection failed, rc="));
    Serial.println(client.state());
    #endif
  }
  wifiClient.setTimeout(5000);
}

void MQTTManager::publishScheduleResult(bool success, const char* message) {
  if (!client.connected()) return;
  char topic[64];
  snprintf(topic, sizeof(topic), "%s/schedule/result", config->prefix);
  StaticJsonDocument<128> doc;
  doc["success"] = success;
  doc["message"] = message;
  String payload;
  serializeJson(doc, payload);
  client.publish(topic, payload.c_str(), false);
}

void MQTTManager::publishSchedule() {
  if (!client.connected()) return;
  char topic[64];
  snprintf(topic, sizeof(topic), "%s/schedule", config->prefix);
  
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
  String payload;
  serializeJson(doc, payload);
  // Важное расписание публикуем с retain
  client.publish(topic, payload.c_str(), true);
}

void MQTTManager::callback(char* topic, byte* payload, unsigned int length) {
  static char msg[MAX_JSON_LARGE + 1];
  if (length > MAX_JSON_LARGE) {
    length = MAX_JSON_LARGE;
    #ifdef DEBUG_ENABLED
    Serial.println(F("MQTT message truncated to MAX_JSON_LARGE"));
    #endif
  }
  memcpy(msg, payload, length);
  msg[length] = '\0';
  
  #ifdef DEBUG_ENABLED
  Serial.print(F("MQTT message ["));
  Serial.print(topic);
  Serial.print(F("] "));
  Serial.println(msg);
  #endif

  char cmpTopic[64];
  
  snprintf(cmpTopic, sizeof(cmpTopic), "%s/set", config->prefix);
  if (strcmp(topic, cmpTopic) == 0) {
    if (strcmp(msg, "ON") == 0) {
      String error;
      if (!StartGuard::canStart(StartMode::PUMP_ONLY, error)) {
        #ifdef DEBUG_ENABLED
        Serial.print(F("MQTT ON blocked: "));
        Serial.println(error);
        #endif
        return;
      }
      pumpOn();
    } else if (strcmp(msg, "OFF") == 0) {
      if (TaskRunner::isRunning()) {
        TaskRunner::stop();
      }
      pumpOff();
    }
    return;
  }

  snprintf(cmpTopic, sizeof(cmpTopic), "%s/calibrate/volume/set", config->prefix);
  if (strcmp(topic, cmpTopic) == 0) {
    float vol = atof(msg);
    if (vol >= 1 && vol <= 10000) {
      CalibrationManager::setTargetVolume(vol);
      #ifdef DEBUG_ENABLED
      Serial.print(F("Calibration target volume set to: "));
      Serial.println(vol);
      #endif
      snprintf(cmpTopic, sizeof(cmpTopic), "%s/calibrate/volume", config->prefix);
      client.publish(cmpTopic, msg, true);
    }
    return;
  }

  snprintf(cmpTopic, sizeof(cmpTopic), "%s/calibrate/start", config->prefix);
  if (strcmp(topic, cmpTopic) == 0) {
    if (SystemStatus::isFixedCalibration()) {
      #ifdef DEBUG_ENABLED
      Serial.println(F("MQTT calibrate/start blocked: fixed calibration active"));
      #endif
      return;
    }
    String error;
    if (!StartGuard::canStart(StartMode::CALIBRATE, error)) {
      #ifdef DEBUG_ENABLED
      Serial.print(F("MQTT calibrate blocked: "));
      Serial.println(error);
      #endif
      return;
    }
    float vol = CalibrationManager::getTargetVolume();
    pumpOn();
    SystemStatus::setCalibrating(true);
    SystemStatus::setCalibrationStart(micros());
    SystemStatus::setCalibrationTarget(vol);
    SystemStatus::setFixedCalibration(false);
    publishCalibrationStatus(true, vol, 0, CalibrationManager::isCalibrated());
    #ifdef DEBUG_ENABLED
    Serial.println(F("Calibration started via MQTT"));
    #endif
    return;
  }

  snprintf(cmpTopic, sizeof(cmpTopic), "%s/calibrate/stop", config->prefix);
  if (strcmp(topic, cmpTopic) == 0) {
    if (SystemStatus::isFixedCalibration()) {
      #ifdef DEBUG_ENABLED
      Serial.println(F("MQTT calibrate/stop blocked: fixed calibration active"));
      #endif
      return;
    }
    if (SystemStatus::isCalibrating()) {
      pumpOff();
      unsigned long elapsedMicros = micros() - SystemStatus::getCalibrationStart();
      float seconds = elapsedMicros / 1000000.0f;
      float flowRate = SystemStatus::getCalibrationTarget() / seconds;
      CalibrationManager::setFlowRate(flowRate);
      SystemStatus::resetCalibration();
      publishCalibrationStatus(false, 0, 0, CalibrationManager::isCalibrated());
      #ifdef DEBUG_ENABLED
      Serial.println(F("Calibration stopped via MQTT"));
      #endif
    }
    return;
  }

  snprintf(cmpTopic, sizeof(cmpTopic), "%s/dispense/volume/set", config->prefix);
  if (strcmp(topic, cmpTopic) == 0) {
    float vol = atof(msg);
    if (vol >= 1 && vol <= 10000) {
      dispenseVolume = vol;
      #ifdef DEBUG_ENABLED
      Serial.print(F("Dispense volume set to: "));
      Serial.println(vol);
      #endif
      snprintf(cmpTopic, sizeof(cmpTopic), "%s/dispense/volume", config->prefix);
      client.publish(cmpTopic, msg, true);
    }
    return;
  }

  snprintf(cmpTopic, sizeof(cmpTopic), "%s/dispense/run", config->prefix);
  if (strcmp(topic, cmpTopic) == 0) {
    float flowRate = CalibrationManager::getFlowRate();
    if (flowRate <= 0 || dispenseVolume <= 0) {
      #ifdef DEBUG_ENABLED
      Serial.println(F("Cannot dispense: flow rate not calibrated or volume invalid"));
      #endif
      return;
    }
    String error;
    if (!StartGuard::canStart(StartMode::DISPENSE, error)) {
      #ifdef DEBUG_ENABLED
      Serial.print(F("MQTT dispense blocked: "));
      Serial.println(error);
      #endif
      return;
    }
    unsigned long runTimeMs = (unsigned long)((dispenseVolume / flowRate) * 1000);
    TaskRunner::start(runTimeMs, (int)dispenseVolume);
    #ifdef DEBUG_ENABLED
    Serial.print(F("Dispensing started for volume: "));
    Serial.println(dispenseVolume);
    #endif
    return;
  }

  snprintf(cmpTopic, sizeof(cmpTopic), "%s/dispense/stop", config->prefix);
  if (strcmp(topic, cmpTopic) == 0) {
    if (TaskRunner::isRunning()) {
      TaskRunner::stop();
      #ifdef DEBUG_ENABLED
      Serial.println(F("Dispensing stopped via MQTT"));
      #endif
    } else {
      #ifdef DEBUG_ENABLED
      Serial.println(F("No dispense task running"));
      #endif
    }
    return;
  }

  snprintf(cmpTopic, sizeof(cmpTopic), "%s/schedule/get", config->prefix);
  if (strcmp(topic, cmpTopic) == 0) {
    publishSchedule();
    return;
  }

  snprintf(cmpTopic, sizeof(cmpTopic), "%s/schedule/add", config->prefix);
  if (strcmp(topic, cmpTopic) == 0) {
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, msg);
    if (error) {
      publishScheduleResult(false, "Invalid JSON");
      return;
    }
    ScheduleJob job;
    job.hour = doc["hour"] | 255;
    job.minute = doc["minute"] | 255;
    job.daysMask = doc["daysMask"] | 0;
    job.volume = doc["volume"] | -1;
    job.enabled = doc["enabled"] | true;
    
    if (job.hour > 23 || job.minute > 59 || job.volume <= 0) {
      publishScheduleResult(false, "Invalid values");
      return;
    }

    if (job.daysMask == 0) {
      publishScheduleResult(false, "At least one day must be selected");
      return;
    }

    if (!CalibrationManager::isCalibrated()) {
      publishScheduleResult(false, "Calibration required");
      return;
    }

    if (SchedulerManager::getJobCount() >= MAX_SCHEDULE_JOBS) {
      publishScheduleResult(false, "Maximum jobs limit reached");
      return;
    }
    
    if (!SchedulerManager::isTimeSlotAvailable(job)) {
      publishScheduleResult(false, "Time slot already occupied");
      return;
    }
    
    if (SchedulerManager::addJob(job)) {
      publishScheduleResult(true, "Job added");
      publishSchedule();
    } else {
      publishScheduleResult(false, "Failed to save");
    }
    return;
  }

  snprintf(cmpTopic, sizeof(cmpTopic), "%s/schedule/remove", config->prefix);
  if (strcmp(topic, cmpTopic) == 0) {
    StaticJsonDocument<64> doc;
    DeserializationError error = deserializeJson(doc, msg);
    if (error || !doc.containsKey("id")) {
      publishScheduleResult(false, "Invalid JSON or missing id");
      return;
    }
    int id = doc["id"];
    if (SchedulerManager::removeJob(id)) {
      publishScheduleResult(true, "Job removed");
      publishSchedule();
    } else {
      publishScheduleResult(false, "Job not found");
    }
    return;
  }

  snprintf(cmpTopic, sizeof(cmpTopic), "%s/schedule/update", config->prefix);
  if (strcmp(topic, cmpTopic) == 0) {
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, msg);
    if (error || !doc.containsKey("id")) {
      publishScheduleResult(false, "Invalid JSON or missing id");
      return;
    }
    int id = doc["id"];
    ScheduleJob job;
    bool found = false;
    for (int i = 0; i < SchedulerManager::getJobCount(); i++) {
      job = SchedulerManager::getJob(i);
      if (job.id == id) {
        found = true;
        break;
      }
    }
    if (!found) {
      publishScheduleResult(false, "Job not found");
      return;
    }
    if (doc.containsKey("hour")) job.hour = doc["hour"];
    if (doc.containsKey("minute")) job.minute = doc["minute"];
    if (doc.containsKey("daysMask")) job.daysMask = doc["daysMask"];
    if (doc.containsKey("volume")) job.volume = doc["volume"];
    if (doc.containsKey("enabled")) job.enabled = doc["enabled"];
    
    if (job.hour > 23 || job.minute > 59 || job.volume <= 0) {
      publishScheduleResult(false, "Invalid values");
      return;
    }

    if (job.daysMask == 0) {
      publishScheduleResult(false, "At least one day must be selected");
      return;
    }

    if (doc.containsKey("volume") && !CalibrationManager::isCalibrated()) {
      publishScheduleResult(false, "Calibration required");
      return;
    }

    for (int i = 0; i < SchedulerManager::getJobCount(); i++) {
      const ScheduleJob& j = SchedulerManager::getJob(i);
      if (j.id != id && j.hour == job.hour && j.minute == job.minute && (j.daysMask & job.daysMask) != 0) {
        publishScheduleResult(false, "Time slot conflict");
        return;
      }
    }
    
    if (SchedulerManager::updateJob(job)) {
      publishScheduleResult(true, "Job updated");
      publishSchedule();
    } else {
      publishScheduleResult(false, "Failed to save");
    }
    return;
  }

  snprintf(cmpTopic, sizeof(cmpTopic), "%s/brightness/set", config->prefix);
  if (strcmp(topic, cmpTopic) == 0) {
    int percent = atoi(msg);
    if (percent >= 0 && percent <= 100) {
      uint8_t val = (uint8_t)(percent * 255 / 100);
      DeviceConfig cfg;
      loadDeviceConfig(cfg);
      cfg.ledBrightness = val;
      if (saveDeviceConfig(cfg)) {
        LedIndicator::setBrightness(val);
        publishBrightness();
      }
    }
    return;
  }
}

void MQTTManager::publishState(bool pumpOn) {
  if (!client.connected()) return;
  char topic[64];
  snprintf(topic, sizeof(topic), "%s/state", config->prefix);
  client.publish(topic, pumpOn ? "ON" : "OFF", true);  // retain
  #ifdef DEBUG_ENABLED
  Serial.print(F("Published state: "));
  Serial.println(pumpOn ? "ON" : "OFF");
  #endif
}

void MQTTManager::publishLevel(bool levelOk) {
  if (!client.connected()) return;
  char topic[64];
  snprintf(topic, sizeof(topic), "%s/level", config->prefix);
  client.publish(topic, levelOk ? "OK" : "LOW", true);  // retain
  #ifdef DEBUG_ENABLED
  Serial.print(F("Published level: "));
  Serial.println(levelOk ? "OK" : "LOW");
  #endif
}

void MQTTManager::publishAvailability(bool online) {
  if (!client.connected()) return;
  char topic[64];
  snprintf(topic, sizeof(topic), "%s/availability", config->prefix);
  client.publish(topic, online ? "online" : "offline", true);  // retain
  #ifdef DEBUG_ENABLED
  Serial.print(F("Published availability: "));
  Serial.println(online ? "online" : "offline");
  #endif
}

void MQTTManager::publishCalibrationStatus(bool calibrating, float target, float elapsed, bool calibrated) {
  if (!client.connected()) return;
  char topic[64];
  snprintf(topic, sizeof(topic), "%s/calibrate/state", config->prefix);
  
  StaticJsonDocument<192> doc;
  doc["calibrating"] = calibrating;
  doc["calibrated"] = calibrated;
  doc["targetVolume"] = target;
  doc["elapsedSec"] = elapsed;
  doc["flowRate"] = CalibrationManager::getFlowRate();
  doc["fixed"] = SystemStatus::isFixedCalibration();
  doc["remainingSec"] = SystemStatus::getFixedCalibrationRemaining();
  String payload;
  serializeJson(doc, payload);
  client.publish(topic, payload.c_str(), true);  // retain
  
  snprintf(topic, sizeof(topic), "%s/calibrating", config->prefix);
  client.publish(topic, calibrating ? "ON" : "OFF", true);  // retain
  
  #ifdef DEBUG_ENABLED
  Serial.print(F("Published calibration status: "));
  Serial.println(payload);
  #endif
}

bool MQTTManager::isConnected() {
  return client.connected();
}

void MQTTManager::publishDispenseFinished(bool success, int volume, unsigned long elapsedMs) {
  if (!client.connected()) return;
  char topic[64];
  snprintf(topic, sizeof(topic), "%s/dispense/finished", config->prefix);
  StaticJsonDocument<128> doc;
  doc["success"] = success;
  doc["volume"] = volume;
  doc["elapsedMs"] = elapsedMs;
  String payload;
  serializeJson(doc, payload);
  client.publish(topic, payload.c_str(), false);  // без retain
  #ifdef DEBUG_ENABLED
  Serial.print(F("Published dispense finished: "));
  Serial.println(payload);
  #endif
}

void MQTTManager::publishDiscovery() {
  sendDiscovery();
}

void MQTTManager::sendDiscovery() {
  if (!client.connected()) return;
  
  String deviceId = getDeviceId();
  String nodeId = deviceId;
  String baseTopic = "homeassistant";
  
  StaticJsonDocument<512> deviceDoc;
  deviceDoc["identifiers"][0] = deviceId;
  deviceDoc["name"] = "AFD (Automatic Fertilizer Dispenser)";
  deviceDoc["sw_version"] = VERSION;
  deviceDoc["model"] = "ESP8266 Pump";
  deviceDoc["manufacturer"] = "Custom";

  char topic[128];
  char payload[2048];

  snprintf(topic, sizeof(topic), "%s/switch/%s/pump/config", baseTopic.c_str(), nodeId.c_str());
  DynamicJsonDocument switchDoc(1024);
  switchDoc["name"] = "Pump";
  switchDoc["command_topic"] = String(config->prefix) + "/set";
  switchDoc["state_topic"] = String(config->prefix) + "/state";
  switchDoc["availability_topic"] = String(config->prefix) + "/availability";
  switchDoc["payload_on"] = "ON";
  switchDoc["payload_off"] = "OFF";
  switchDoc["state_on"] = "ON";
  switchDoc["state_off"] = "OFF";
  switchDoc["unique_id"] = deviceId + "_pump";
  switchDoc["device"] = deviceDoc.as<JsonObject>();
  serializeJson(switchDoc, payload, sizeof(payload));
  client.publish(topic, payload, true);

  snprintf(topic, sizeof(topic), "%s/binary_sensor/%s/level/config", baseTopic.c_str(), nodeId.c_str());
  DynamicJsonDocument levelDoc(1024);
  levelDoc["name"] = "Water Level";
  levelDoc["state_topic"] = String(config->prefix) + "/level";
  levelDoc["availability_topic"] = String(config->prefix) + "/availability";
  levelDoc["payload_on"] = "LOW";
  levelDoc["payload_off"] = "OK";
  levelDoc["device_class"] = "problem";
  levelDoc["unique_id"] = deviceId + "_level";
  levelDoc["device"] = deviceDoc.as<JsonObject>();
  serializeJson(levelDoc, payload, sizeof(payload));
  client.publish(topic, payload, true);

  snprintf(topic, sizeof(topic), "%s/binary_sensor/%s/calibrating/config", baseTopic.c_str(), nodeId.c_str());
  DynamicJsonDocument calDoc(1024);
  calDoc["name"] = "Calibrating";
  calDoc["state_topic"] = String(config->prefix) + "/calibrating";
  calDoc["availability_topic"] = String(config->prefix) + "/availability";
  calDoc["payload_on"] = "ON";
  calDoc["payload_off"] = "OFF";
  calDoc["device_class"] = "running";
  calDoc["unique_id"] = deviceId + "_calibrating";
  calDoc["device"] = deviceDoc.as<JsonObject>();
  serializeJson(calDoc, payload, sizeof(payload));
  client.publish(topic, payload, true);

  snprintf(topic, sizeof(topic), "%s/sensor/%s/flow_rate/config", baseTopic.c_str(), nodeId.c_str());
  DynamicJsonDocument flowDoc(1024);
  flowDoc["name"] = "Flow Rate";
  flowDoc["state_topic"] = String(config->prefix) + "/calibrate/state";
  flowDoc["availability_topic"] = String(config->prefix) + "/availability";
  flowDoc["unit_of_measurement"] = "ml/s";
  flowDoc["value_template"] = "{{ value_json.flowRate }}";
  flowDoc["unique_id"] = deviceId + "_flow_rate";
  flowDoc["device"] = deviceDoc.as<JsonObject>();
  serializeJson(flowDoc, payload, sizeof(payload));
  client.publish(topic, payload, true);

  snprintf(topic, sizeof(topic), "%s/number/%s/brightness/config", baseTopic.c_str(), nodeId.c_str());
  DynamicJsonDocument brightDoc(1024);
  brightDoc["name"] = "LED Brightness";
  brightDoc["command_topic"] = String(config->prefix) + "/brightness/set";
  brightDoc["state_topic"] = String(config->prefix) + "/brightness";
  brightDoc["availability_topic"] = String(config->prefix) + "/availability";
  brightDoc["min"] = 0;
  brightDoc["max"] = 100;
  brightDoc["step"] = 1;
  brightDoc["unit_of_measurement"] = "%";
  brightDoc["unique_id"] = deviceId + "_brightness";
  brightDoc["device"] = deviceDoc.as<JsonObject>();
  serializeJson(brightDoc, payload, sizeof(payload));
  client.publish(topic, payload, true);

  snprintf(topic, sizeof(topic), "%s/number/%s/dispense_volume/config", baseTopic.c_str(), nodeId.c_str());
  DynamicJsonDocument volumeDoc(1024);
  volumeDoc["name"] = "Dispense Volume";
  volumeDoc["command_topic"] = String(config->prefix) + "/dispense/volume/set";
  volumeDoc["state_topic"] = String(config->prefix) + "/dispense/volume";
  volumeDoc["availability_topic"] = String(config->prefix) + "/availability";
  volumeDoc["min"] = 1;
  volumeDoc["max"] = 10000;
  volumeDoc["step"] = 1;
  volumeDoc["unit_of_measurement"] = "ml";
  volumeDoc["unique_id"] = deviceId + "_dispense_volume";
  volumeDoc["device"] = deviceDoc.as<JsonObject>();
  serializeJson(volumeDoc, payload, sizeof(payload));
  client.publish(topic, payload, true);

  #ifdef DEBUG_ENABLED
  Serial.println(F("MQTT Discovery sent"));
  #endif
}

void MQTTManager::publishBrightness() {
  if (!client.connected()) return;
  DeviceConfig cfg;
  loadDeviceConfig(cfg);
  uint8_t percent = (cfg.ledBrightness * 100) / 255;
  char topic[64];
  snprintf(topic, sizeof(topic), "%s/brightness", config->prefix);
  client.publish(topic, String(percent).c_str(), true);  // retain
}

void MQTTManager::publishFullState() {
  if (!client.connected()) return;
  #ifdef DEBUG_ENABLED
  Serial.println(F("Publishing full MQTT state"));
  #endif
  publishState(isPumpOn());
  publishLevel(LevelSensor::isLevelOk());
  publishCalibrationStatus(SystemStatus::isCalibrating(),
                           CalibrationManager::getTargetVolume(),
                           (SystemStatus::isCalibrating() ? (micros() - SystemStatus::getCalibrationStart()) / 1000000.0f : 0),
                           CalibrationManager::isCalibrated());
  publishBrightness();
  publishSchedule();
  
  char topic[64];
  snprintf(topic, sizeof(topic), "%s/dispense/volume", config->prefix);
  client.publish(topic, String(dispenseVolume).c_str(), true);
  
  snprintf(topic, sizeof(topic), "%s/calibrate/volume", config->prefix);
  client.publish(topic, String(CalibrationManager::getTargetVolume()).c_str(), true);
}