#include "DisplayManager.h"
#include "SystemStatus.h"
#include "LevelSensor.h"
#include "TaskRunner.h"
#include "CalibrationManager.h"
#include "PumpControl.h"
#include "Settings.h"
#include "version.h"
#include <Wire.h>

hd44780_I2Cexp DisplayManager::lcd;
String DisplayManager::_currentIP = "No IP";
String DisplayManager::_currentTime = "--:--:--";
String DisplayManager::_currentDate = "--.--";
String DisplayManager::_lastFirstLine = "";
String DisplayManager::_lastSecondLine = "";
unsigned long DisplayManager::_lastUpdate = 0;

bool DisplayManager::begin() {
  Wire.begin();
  
  delay(1000);
  
  int status = lcd.begin(LCD_COLS, LCD_ROWS);
  if (status) {
    #ifdef DEBUG_ENABLED
    Serial.print(F("LCD begin failed, error: "));
    Serial.println(status);
    #endif
    return false;
  }
  
  delay(100);
  lcd.setCursor(0, 0);
  delay(50);
  
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  delay(100);
  lcd.print(F("Initializing..."));
  lcd.setCursor(0, 1);
  lcd.print("V " VERSION);
  _lastFirstLine = "Initializing...";
  _lastSecondLine = "V " + String(VERSION);
  return true;
}

void DisplayManager::setIP(const String& ip) {
  _currentIP = ip;
}

void DisplayManager::setTime(const String& time) {
  _currentTime = time;
}

void DisplayManager::setDate(const String& date) {
  _currentDate = date;
}

String DisplayManager::buildFirstLine() {
  if (SystemStatus::isCalibrating()) {
    float target = SystemStatus::getCalibrationTarget();
    if (target == 0) {
      return "CAL FIX";
    } else {
      return "CAL " + String(target, 0) + " ml";
    }
  }
  if (TaskRunner::isRunning()) {
    return "DISP " + String(TaskRunner::getCurrentVolume()) + " ml";
  }
  if (isPumpOn() && !TaskRunner::isRunning()) {
    return "PUMP ON";
  }
  return _currentIP;
}

String DisplayManager::buildSecondLine() {
  if (SystemStatus::isCalibrating()) {
    if (SystemStatus::isFixedCalibration() && !isPumpOn()) {
      unsigned long elapsed = SystemStatus::getFixedCalibrationElapsed();
      if (elapsed == 0) {
        elapsed = (micros() - SystemStatus::getCalibrationStart()) / 1000000;
      }
      return String(elapsed) + " s";
    } else {
      unsigned long elapsed = (micros() - SystemStatus::getCalibrationStart()) / 1000000;
      return String(elapsed) + " s";
    }
  }
  if (TaskRunner::isRunning()) {
    unsigned long elapsed = millis() - TaskRunner::getStartTime();
    unsigned long total = TaskRunner::getDuration();
    unsigned long remaining = (total > elapsed) ? (total - elapsed) / 1000 : 0;
    return "Rem: " + String(remaining) + " s";
  }
  if (LevelSensor::isLowLevel()) {
    return "LOW LEVEL!";
  }
  // Формат: HH:MM:SS   DD.MM (три пробела между временем и датой)
  String datetime = _currentTime + "   " + _currentDate;
  if (datetime.length() > LCD_COLS) {
    datetime = datetime.substring(0, LCD_COLS);
  }
  return datetime;
}

void DisplayManager::display() {
  String firstLine = buildFirstLine();
  String secondLine = buildSecondLine();

  if (firstLine.length() > LCD_COLS) firstLine = firstLine.substring(0, LCD_COLS);
  if (secondLine.length() > LCD_COLS) secondLine = secondLine.substring(0, LCD_COLS);

  bool updateFirst = (firstLine != _lastFirstLine);
  bool updateSecond = (secondLine != _lastSecondLine);

  if (updateFirst) {
    lcd.setCursor(0, 0);
    delay(10);
    lcd.print(firstLine);
    if (firstLine.length() < _lastFirstLine.length()) {
      for (int i = firstLine.length(); i < LCD_COLS; i++) lcd.print(' ');
    }
    _lastFirstLine = firstLine;
  }

  if (updateSecond) {
    lcd.setCursor(0, 1);
    delay(10);
    lcd.print(secondLine);
    if (secondLine.length() < _lastSecondLine.length()) {
      for (int i = secondLine.length(); i < LCD_COLS; i++) lcd.print(' ');
    }
    _lastSecondLine = secondLine;
  }
}

void DisplayManager::update() {
  unsigned long now = millis();
  if (now - _lastUpdate >= LCD_UPDATE_INTERVAL) {
    _lastUpdate = now;
    display();
  }
}