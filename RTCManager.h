#ifndef RTCMANAGER_H
#define RTCMANAGER_H

#include <Arduino.h>
#include <RTClib.h>

class RTCManager {
public:
  static bool begin();
  static DateTime now();
  static bool isRunning();
  static String getTimeString();
  static String getDateString();
  static String getShortDateString();   // новый метод
  static void setDateTime(const DateTime& dt);

private:
  static RTC_DS1307 rtc;
  static bool initialized;
};

#endif