#ifndef SMART_WIFI_H
#define SMART_WIFI_H

#include "Utils.h"
#include "SDHandler.h"
#include <WiFi.h>

#define WIFI_DETAILS_FILE "/wifiDetails.bin"
#define WIFI_MAX_ATTEMPTS 3

struct WifiDetails {
    String ssid;
    String password;
};

class SmartWifi {
public:
    SmartWifi() : wifiHandler(WIFI_DETAILS_FILE), smartWait(500, SmartWait::MILLIS), connectionAttempts(0) {}

    // Non-blocking connect
    // TODO: Wifi.being will take a sec to connect. make it multithreaded
    bool connect() {
        if (smartWait.timePassed()) {
            // If not connected, start connection
            bool connected = isConnected();
            if (connected) {
                LOG("already connected to WiFi");
                return connected;
            } else {
                LOG("connecting to WiFi...");
                WiFi.begin(wifiInfo.ssid.c_str(), wifiInfo.password.c_str());

                // Check again after starting connection
                connected = isConnected();
                if (!connected)
                    connectionAttempts++;
                if (maxAttemptsReached())
                    LOG("max wifi connection attempts reached [%d]", connectionAttempts);
            }
            if (connected)
                LOG("connected to WiFi");
            else
                LOG("failed to connect to WiFi");
        }
        return isConnected();
    }

    bool isConnected() {
        return WiFi.status() == WL_CONNECTED;
    }

    void disconnect() {
        WiFi.disconnect();
        LOG("disconnected from WiFi");
    }

    bool maxAttemptsReached() {
        return connectionAttempts >= WIFI_MAX_ATTEMPTS;
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

private:
    WifiDetails wifiInfo;
    SDHandler<WifiDetails> wifiHandler;
    SmartWait smartWait;
    uint32_t connectionAttempts;
};

#endif
