#include "SchedulerManager.h"
#include "CalibrationManager.h"
#include "TaskRunner.h"
#include "RTCManager.h"
#include "SystemStatus.h"
#include "LevelSensor.h"
#include "StartGuard.h"
#include "Settings.h"
#include "ConfigManager.h"   // для hasEnoughSpace
#include <LittleFS.h>
#include <ArduinoJson.h>

ScheduleJob SchedulerManager::jobs[MAX_SCHEDULE_JOBS];
int SchedulerManager::jobCount = 0;
int SchedulerManager::nextId = 1;
const char* SchedulerManager::filename = "/schedule.json";

bool SchedulerManager::load() {
  if (!LittleFS.exists(filename)) {
    jobCount = 0;
    return true;
  }
  File file = LittleFS.open(filename, "r");
  if (!file) return false;
  
  DynamicJsonDocument doc(10000);
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  if (error) return false;
  
  JsonArray arr = doc.as<JsonArray>();
  jobCount = 0;
  int maxId = 0;
  for (JsonObject obj : arr) {
    if (jobCount >= MAX_SCHEDULE_JOBS) break;
    ScheduleJob job;
    job.id = obj["id"] | 0;
    job.hour = obj["hour"] | 0;
    job.minute = obj["minute"] | 0;
    job.daysMask = obj["daysMask"] | 0;
    job.volume = obj["volume"] | 0;
    job.enabled = obj["enabled"] | true;
    job.lastRunTimestamp = 0;
    jobs[jobCount++] = job;
    if (job.id > maxId) maxId = job.id;
  }
  nextId = maxId + 1;
  return true;
}

bool SchedulerManager::save() {
  DynamicJsonDocument doc(10000);
  JsonArray arr = doc.to<JsonArray>();
  for (int i = 0; i < jobCount; i++) {
    JsonObject obj = arr.createNestedObject();
    obj["id"] = jobs[i].id;
    obj["hour"] = jobs[i].hour;
    obj["minute"] = jobs[i].minute;
    obj["daysMask"] = jobs[i].daysMask;
    obj["volume"] = jobs[i].volume;
    obj["enabled"] = jobs[i].enabled;
  }
  size_t jsonSize = measureJson(doc);
  if (!hasEnoughSpace(jsonSize)) {
    #ifdef DEBUG_ENABLED
    Serial.println(F("Schedule save failed: insufficient space"));
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

int SchedulerManager::getJobCount() {
  return jobCount;
}

const ScheduleJob& SchedulerManager::getJob(int index) {
  return jobs[index];
}

bool SchedulerManager::addJob(const ScheduleJob& job) {
  if (jobCount >= MAX_SCHEDULE_JOBS) return false;
  ScheduleJob newJob = job;
  newJob.id = nextId++;
  newJob.lastRunTimestamp = 0;
  jobs[jobCount++] = newJob;
  return save();
}

bool SchedulerManager::removeJob(int id) {
  int index = -1;
  for (int i = 0; i < jobCount; i++) {
    if (jobs[i].id == id) {
      index = i;
      break;
    }
  }
  if (index == -1) return false;
  
  for (int i = index; i < jobCount - 1; i++) {
    jobs[i] = jobs[i + 1];
  }
  jobCount--;
  return save();
}

bool SchedulerManager::updateJob(const ScheduleJob& job) {
  int index = -1;
  for (int i = 0; i < jobCount; i++) {
    if (jobs[i].id == job.id) {
      index = i;
      break;
    }
  }
  if (index == -1) return false;
  jobs[index] = job;
  jobs[index].lastRunTimestamp = 0;
  return save();
}

bool SchedulerManager::isTimeSlotAvailable(const ScheduleJob& newJob) {
  for (int i = 0; i < jobCount; i++) {
    const ScheduleJob& existing = jobs[i];
    if (existing.hour == newJob.hour && existing.minute == newJob.minute) {
      if ((existing.daysMask & newJob.daysMask) != 0) {
        return false;
      }
    }
  }
  return true;
}

static bool dayMatchesMask(uint8_t mask) {
  if (!RTCManager::isRunning()) return false;
  DateTime now = RTCManager::now();
  int dow = now.dayOfTheWeek();
  uint8_t bit = 0;
  switch (dow) {
    case 1: bit = 1; break;
    case 2: bit = 2; break;
    case 3: bit = 4; break;
    case 4: bit = 8; break;
    case 5: bit = 16; break;
    case 6: bit = 32; break;
    case 0: bit = 64; break;
  }
  return (mask & bit) != 0;
}

void SchedulerManager::checkAndRun() {
  if (!RTCManager::isRunning()) return;
  
  DateTime now = RTCManager::now();
  if (now.year() < 2000) return;
  
  if (SystemStatus::isCalibrating()) return;
  if (TaskRunner::isRunning()) return;
  
  unsigned long currentUnix = now.unixtime();
  float flowRate = CalibrationManager::getFlowRate();
  if (flowRate <= 0) return;
  
  for (int i = 0; i < jobCount; i++) {
    ScheduleJob& job = jobs[i];
    if (!job.enabled) continue;
    if (job.hour == now.hour() && job.minute == now.minute()) {
      if (!dayMatchesMask(job.daysMask)) continue;
      
      if (currentUnix - job.lastRunTimestamp < 60) continue;
      
      String error;
      if (!StartGuard::canStart(StartMode::DISPENSE, error)) {
        #ifdef DEBUG_ENABLED
        Serial.print(F("Scheduler blocked job "));
        Serial.print(job.id);
        Serial.print(F(": "));
        Serial.println(error);
        #endif
        continue;
      }
      
      unsigned long runTimeMs = (unsigned long)((job.volume / flowRate) * 1000);
      if (runTimeMs == 0) continue;
      
      TaskRunner::start(runTimeMs, job.volume);
      job.lastRunTimestamp = currentUnix;
      
      #ifdef DEBUG_ENABLED
      Serial.printf_P(PSTR("Scheduler: started job %d, volume %d ml, runtime %lu ms\n"), job.id, job.volume, runTimeMs);
      #endif
    }
  }
}