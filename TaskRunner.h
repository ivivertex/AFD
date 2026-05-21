#ifndef TASKRUNNER_H
#define TASKRUNNER_H

#include <Arduino.h>

class TaskRunner {
public:
  static void start(unsigned long durationMs, int volume);
  static void stop();
  static bool isRunning();
  static void update();
  
  static unsigned long getStartTime();
  static unsigned long getDuration();
  static int getCurrentVolume();

private:
  static bool running;
  static unsigned long stopTime;
  static unsigned long startTime;
  static unsigned long durationMs;
  static int currentVolume;
};

#endif