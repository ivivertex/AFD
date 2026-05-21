#ifndef SCHEDULERMANAGER_H
#define SCHEDULERMANAGER_H

#include <Arduino.h>
#include "Settings.h"

struct ScheduleJob {
  int id;               // уникальный идентификатор
  uint8_t hour;         // час (0-23)
  uint8_t minute;       // минута (0-59)
  uint8_t daysMask;     // битовая маска: 1=пн, 2=вт, 4=ср, 8=чт, 16=пт, 32=сб, 64=вс
  int volume;           // объём в мл
  bool enabled;         // активна ли задача
  unsigned long lastRunTimestamp; // Unix timestamp последнего запуска (runtime, не сохраняется)
};

class SchedulerManager {
public:
  static bool load();                           // загрузить задачи из файла
  static bool save();                            // сохранить задачи в файл
  static int getJobCount();                       // получить количество заданий
  static const ScheduleJob& getJob(int index);    // получить задание по индексу
  static bool addJob(const ScheduleJob& job);     // добавить задачу (id генерируется)
  static bool removeJob(int id);                  // удалить задачу по id
  static bool updateJob(const ScheduleJob& job);  // обновить существующее задание
  static void checkAndRun();                      // проверяет текущее время и запускает задачи
  static bool isTimeSlotAvailable(const ScheduleJob& newJob); // проверка на дублирование времени

private:
  static ScheduleJob jobs[MAX_SCHEDULE_JOBS];
  static int jobCount;
  static int nextId;
  static const char* filename;
};

#endif