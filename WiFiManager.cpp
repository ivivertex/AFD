#include "WiFiManager.h"
#include "ConfigManager.h"
#include "Settings.h"

bool connectToWiFi() {
  WiFiConfig* cfg = getWiFiConfig();
  if (strlen(cfg->ssid) == 0) {
    return false;
  }

  WiFi.mode(WIFI_STA);
  
  if (cfg->useStaticIP) {
    IPAddress ip, gateway, subnet, dns1, dns2;
    bool ipOk = ip.fromString(cfg->staticIP);
    bool gwOk = gateway.fromString(cfg->gateway);
    bool snOk = subnet.fromString(cfg->subnet);
    
    if (ipOk && gwOk && snOk) {
      WiFi.config(ip, gateway, subnet);
      
      if (strlen(cfg->dns1) > 0 && dns1.fromString(cfg->dns1)) {
        if (strlen(cfg->dns2) > 0 && dns2.fromString(cfg->dns2)) {
          WiFi.config(ip, gateway, subnet, dns1, dns2);
        } else {
          WiFi.config(ip, gateway, subnet, dns1);
        }
      }
      #ifdef DEBUG_ENABLED
      Serial.println(F("Static IP configured"));
      #endif
    } else {
      #ifdef DEBUG_ENABLED
      Serial.println(F("Invalid static IP settings, using DHCP"));
      #endif
    }
  }

  WiFi.begin(cfg->ssid, cfg->password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    #ifdef DEBUG_ENABLED
    Serial.print(".");
    #endif
    attempts++;
  }
  #ifdef DEBUG_ENABLED
  Serial.println();
  #endif

  if (WiFi.status() == WL_CONNECTED) {
    #ifdef DEBUG_ENABLED
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    #endif
    return true;
  } else {
    #ifdef DEBUG_ENABLED
    Serial.println("WiFi connection failed");
    #endif
    return false;
  }
}

void startAPMode() {
  WiFi.mode(WIFI_AP);
  WiFiConfig* cfg = getWiFiConfig();
  const char* apPassword = (strlen(cfg->apPassword) > 0) ? cfg->apPassword : AP_PASSWORD;
  WiFi.softAP(AP_SSID, apPassword);
  #ifdef DEBUG_ENABLED
  Serial.println("AP Mode started");
  Serial.print("AP SSID: ");
  Serial.println(AP_SSID);
  Serial.print("AP password: ");
  Serial.println(apPassword);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  #endif
}

bool isWiFiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

String getSTAIP() {
  if (isWiFiConnected()) {
    return WiFi.localIP().toString();
  }
  return "";
}

String getAPIP() {
  if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
    return WiFi.softAPIP().toString();
  }
  return "";
}

String getDisplayIP() {
  if (isWiFiConnected()) {
    return getSTAIP();
  } else if (WiFi.getMode() == WIFI_AP) {
    return getAPIP();
  } else {
    return "No IP";
  }
}

bool isAPMode() {
  return WiFi.getMode() == WIFI_AP;
}