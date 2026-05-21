#include "RTCManager.h"
#include "Settings.h"

RTC_DS1307 RTCManager::rtc;
bool RTCManager::initialized = false;

bool RTCManager::begin() {
  if (!rtc.begin()) {
    initialized = false;
    return false;
  }
  initialized = true;
  return true;
}

DateTime RTCManager::now() {
  if (!initialized) return DateTime(2000, 1, 1, 0, 0, 0);
  return rtc.now();
}

bool RTCManager::isRunning() {
  return initialized && rtc.isrunning();
}

String RTCManager::getTimeString() {
  if (!initialized) return "RTC err";
  DateTime t = now();
  char buf[9];
  sprintf(buf, "%02d:%02d:%02d", t.hour(), t.minute(), t.second());
  return String(buf);
}

String RTCManager::getDateString() {
  if (!initialized) return "--.--.--";
  DateTime t = now();
  char buf[9];
  sprintf(buf, "%02d.%02d.%02d", t.day(), t.month(), t.year() % 100);
  return String(buf);
}

String RTCManager::getShortDateString() {
  if (!initialized) return "--.--";
  DateTime t = now();
  char buf[6];
  sprintf(buf, "%02d.%02d", t.day(), t.month());
  return String(buf);
}

void RTCManager::setDateTime(const DateTime& dt) {
  if (!initialized) return;
  rtc.adjust(dt);
}