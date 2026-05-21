#include "StartGuard.h"
#include "SystemStatus.h"
#include "TaskRunner.h"
#include "LevelSensor.h"
#include "CalibrationManager.h"
#include "PumpControl.h"

bool StartGuard::canStart(StartMode mode, String &errorMsg) {
    // Общие проверки для всех режимов
    if (isPumpOn()) {
        errorMsg = "Pump already on";
        return false;
    }
    if (SystemStatus::isCalibrating()) {
        errorMsg = "Calibration in progress";
        return false;
    }
    if (TaskRunner::isRunning()) {
        errorMsg = "Another task is already running";
        return false;
    }
    if (LevelSensor::isLowLevel()) {
        errorMsg = "Low level - pump blocked";
        return false;
    }

    // Специфичные для режимов
    if (mode == StartMode::DISPENSE) {
        if (!CalibrationManager::isCalibrated()) {
            errorMsg = "Calibration required";
            return false;
        }
    }

    // Для режима CALIBRATE дополнительные проверки не нужны
    return true;
}