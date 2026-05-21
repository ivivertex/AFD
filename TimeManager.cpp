#include "TimeManager.h"
#include "RTCManager.h"
#include "ConfigManager.h"
#include "Settings.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <RTClib.h>

static WiFiUDP udp;
static const int ntpPort = 123;
static const int ntpPacketSize = 48;
static const uint32_t ntpTimeout = 5000;

bool TimeManager::syncNTP() {
  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }

  MQTTConfig* cfg = getMQTTConfig();
  const char* ntpServer = cfg->ntpServer;
  int8_t timezone = cfg->timezone;
  
  IPAddress timeServerIP;
  if (!WiFi.hostByName(ntpServer, timeServerIP)) {
    #ifdef DEBUG_ENABLED
    Serial.print(F("NTP server lookup failed: "));
    Serial.println(ntpServer);
    #endif
    return false;
  }
  
  udp.begin(ntpPort);
  
  byte packetBuffer[ntpPacketSize];
  memset(packetBuffer, 0, ntpPacketSize);
  packetBuffer[0] = 0b11100011;
  packetBuffer[1] = 0;
  packetBuffer[2] = 6;
  packetBuffer[3] = 0xEC;
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  
  udp.beginPacket(timeServerIP, ntpPort);
  udp.write(packetBuffer, ntpPacketSize);
  udp.endPacket();
  
  unsigned long startTime = millis();
  while (millis() - startTime < ntpTimeout) {
    ESP.wdtFeed(); // пункт 6.1
    int size = udp.parsePacket();
    if (size >= ntpPacketSize) {
      udp.read(packetBuffer, ntpPacketSize);
      
      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
      unsigned long secsSince1900 = highWord << 16 | lowWord;
      
      const unsigned long seventyYears = 2208988800UL;
      unsigned long epoch = secsSince1900 - seventyYears;
      
      epoch += timezone * 3600;
      
      DateTime dt = DateTime(epoch);
      RTCManager::setDateTime(dt);
      udp.stop();
      #ifdef DEBUG_ENABLED
      Serial.print(F("NTP sync successful from "));
      Serial.print(ntpServer);
      Serial.print(F(" with timezone "));
      Serial.println(timezone);
      #endif
      return true;
    }
    delay(10);
  }
  udp.stop();
  #ifdef DEBUG_ENABLED
  Serial.println(F("NTP sync timeout"));
  #endif
  return false;
}

bool TimeManager::setDateTime(uint16_t year, uint8_t month, uint8_t day,
                              uint8_t hour, uint8_t minute, uint8_t second) {
  if (year < 2000 || year > 2099 || month < 1 || month > 12 || day < 1 || day > 31 ||
      hour > 23 || minute > 59 || second > 59) {
    return false;
  }
  DateTime dt(year, month, day, hour, minute, second);
  RTCManager::setDateTime(dt);
  return true;
}

String TimeManager::getTimeString() {
  return RTCManager::getTimeString();
}

String TimeManager::getDateString() {
  return RTCManager::getDateString();
}