#ifndef STARTGUARD_H
#define STARTGUARD_H

#include <Arduino.h>

enum class StartMode {
    PUMP_ONLY,      // просто включить насос (без дозирования, без калибровки)
    DISPENSE,       // дозирование (требуется калибровка)
    CALIBRATE       // калибровка (не требуется калибровка)
};

class StartGuard {
public:
    // Проверяет, можно ли запустить операцию.
    // Возвращает true, если все условия соблюдены.
    // В противном случае возвращает false и записывает причину отказа в errorMsg.
    static bool canStart(StartMode mode, String &errorMsg);
};

#endif