#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>

// Раскомментируйте для включения отладочного вывода в Serial
// #define DEBUG_ENABLED

// Пины
const uint8_t BUTTON_PIN = D5;
const uint8_t LEVEL_PIN  = D6;
const uint8_t RESET_BUTTON_PIN = D4;

// WiFi
const char* const AP_SSID = "AFD";
const char* const AP_PASSWORD = "12345678";
const unsigned long WIFI_LOST_TIMEOUT = 30000;      // 30 секунд до перехода в AP режим
const unsigned long WIFI_RETRY_INTERVAL = 300000;   // 5 минут между попытками переподключения

// MQTT
const uint16_t MQTT_DEFAULT_PORT = 1883;
const char* const MQTT_DEFAULT_PREFIX = "AFD";
const int8_t MQTT_DEFAULT_TIMEZONE = 3;

// Калибровка
const unsigned long CALIBRATION_TIMEOUT_US = 10UL * 60 * 1000000;  // 10 минут

// Планировщик
const unsigned long SCHEDULER_CHECK_INTERVAL = 1000;  // Проверка расписания каждую секунду
const int MAX_SCHEDULE_JOBS = 100;                    // Максимум 100 заданий

// Кнопка сброса
const unsigned long RESET_HOLD_TIME = 5000;           // 5 секунд удержания для сброса

// Сенсорная кнопка
const unsigned long BUTTON_DEBOUNCE_DELAY = 50;       // 50 мс антидребезг

// LCD
const uint8_t LCD_ADDRESS = 0x27;
const uint8_t LCD_COLS = 16;
const uint8_t LCD_ROWS = 2;
const unsigned long LCD_UPDATE_INTERVAL = 1000;       // Обновление каждую секунду

// Размеры буферов
const size_t MAX_SSID_LEN = 32;
const size_t MAX_PASS_LEN = 64;
const size_t MAX_IP_STR_LEN = 16;
const size_t MAX_JSON_RESPONSE = 512;
const size_t MAX_JSON_STATUS = 384;                   // Буфер для статуса JSON
const size_t MAX_JSON_LARGE = 4096;

// LED – яркость по умолчанию 
const uint8_t DEFAULT_LED_BRIGHTNESS = 64;

// Перечисления (имена изменены, чтобы избежать конфликта с макросами HIGH/LOW)
enum class LevelActive : uint8_t {
  ACTIVE_LOW = 0,
  ACTIVE_HIGH = 1
};

enum class ButtonActive : uint8_t {
  ACTIVE_LOW = 0,
  ACTIVE_HIGH = 1
};

enum class ResetButtonActive : uint8_t {
  ACTIVE_LOW = 0,
  ACTIVE_HIGH = 1
};

enum class LEDMode : uint8_t {
  NORMAL = 0,
  BLINK = 1,
  RAINBOW = 2
};

#endif