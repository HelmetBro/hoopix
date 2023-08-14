#ifndef SMART_WIFI_H
#define SMART_WIFI_H

#include "Utils.h"
#include "SDHandler.h"
#include <WiFi.h>

#define WIFI_DETAILS_FILE "/wifiDetails.bin"

class WifiDetails {
public:
  String ssid;
  String password;
};

class SmartWifi {
private:
  WifiDetails wifiInfo;
  SDHandler<WifiDetails> wifiHandler;
  SmartWait smartWait;
  uint32_t connectionAttempts;

public:
  SmartWifi() : wifiHandler(WIFI_DETAILS_FILE), smartWait(500, SmartWait::MILLIS), connectionAttempts(0) {}

  // Non-blocking connect
  bool connect() {
    if (smartWait.timePassed()) {
      // If not connected, start connection
      if (WiFi.status() != WL_CONNECTED) {
        WiFi.begin(wifiInfo.ssid.c_str(), wifiInfo.password.c_str());
        
        // Check again after starting connection
        if (WiFi.status() != WL_CONNECTED) connectionAttempts++;
      }
    }

    bool isConnected = WiFi.status() == WL_CONNECTED;

#if DEBUG
    if (isConnected)
      Serial.println("Connected to WiFi");
    else
      Serial.println("Failed to connect to WiFi");
#endif

    return isConnected;
  }

  bool isConnected() {
    return WiFi.status() == WL_CONNECTED;
  }

  void disconnect() {
    WiFi.disconnect();
#if DEBUG
    Serial.println("Disconnected from WiFi");
#endif
  }

  // this is an override, remove and replace
  // later use the encrypted version
  void saveCredentials() {
    if (haveSavedWifi()) {
      wifiHandler.remove();
    }
    wifiHandler.write(wifiInfo);
  }

  bool haveSavedWifi() {
    return wifiHandler.exists();
  }

  uint32_t getConnectionAttempts(){
    return connectionAttempts;
  }
};

#endif
