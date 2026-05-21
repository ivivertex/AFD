#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <ESP8266WiFi.h>

bool connectToWiFi();
void startAPMode();
bool isWiFiConnected();
String getSTAIP();
String getAPIP();
String getDisplayIP();
bool isAPMode();

#endif