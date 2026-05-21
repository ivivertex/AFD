#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

class DisplayManager {
public:
  static bool begin();
  static void update();
  static void setIP(const String& ip);
  static void setTime(const String& time);
  static void setDate(const String& date);

private:
  static hd44780_I2Cexp lcd;
  static String _currentIP;
  static String _currentTime;
  static String _currentDate;
  static String _lastFirstLine;
  static String _lastSecondLine;
  static unsigned long _lastUpdate;
  static void display();
  static String buildFirstLine();
  static String buildSecondLine();
};

#endif