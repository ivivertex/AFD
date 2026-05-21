#ifndef PUMPCONTROL_H
#define PUMPCONTROL_H

#include <Arduino.h>
#include "Settings.h"

#define PUMP_PIN D8

void initPump();
void pumpOn();
void pumpOff();
bool isPumpOn();

#endif