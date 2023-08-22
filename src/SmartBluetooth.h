#ifndef SMART_BLUETOOTH_H
#define SMART_BLUETOOTH_H

#include "SDHandler.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define BLUETOOTH_DEVICE_FILE "/btDevice.bin"

struct DeviceInfo {
    String serviceUUID;
    String characteristicUUID;
    String name;
    std::vector<String> pastDevices;
};

class ServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        LOG("ble server: client connected");
        pServer->getAdvertising()->stop();
    }

    void onDisconnect(BLEServer* pServer) {
        LOG("ble Server: client disconnected");
        pServer->startAdvertising();
    }
};

class CharacteristicCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (value.length() > 0) {
            String receivedValue = "";
            for (int i = 0; i < value.length(); i++) {
                receivedValue += value[i];
            }
            LOG("ble characteristic: received value: [%s]", receivedValue.c_str());
        } else {
            LOG("ble characteristic: received empty value");
        }
    }
};

class SmartBluetooth {
private:
    BLEServer* bleServer;
    BLEService* bleService;
    BLECharacteristic* bleCharacteristic;
    SDHandler<DeviceInfo> deviceHandler;
    DeviceInfo deviceInfo;

    String generateUUID() {
        // https://www.uuidgenerator.net/
        char uuid[37];
        sprintf(uuid, "%08X-%04X-%04X-%04X-%04X%08X",
            static_cast<uint32_t>(esp_random()),
            static_cast<uint16_t>(esp_random()),
            static_cast<uint16_t>(esp_random()),
            static_cast<uint16_t>(esp_random()),
            static_cast<uint16_t>(esp_random()),
            static_cast<uint32_t>(esp_random()));
        return String(uuid);
    }

public:
    SmartBluetooth() : deviceHandler(BLUETOOTH_DEVICE_FILE) {}

    ~SmartBluetooth() {
        if (bleServer) {
            delete bleServer;
            bleServer = nullptr;
        }
        if (bleService) {
            delete bleService;
            bleService = nullptr;
        }
        if (bleCharacteristic) {
            delete bleCharacteristic;
            bleCharacteristic = nullptr;
        }
        LOG("bluetooth destroyed");
    }

    void init() {
        LOG("initializing bluetooth");
        if (!haveSavedDeviceInfo()) {
            LOG("generating new device info");
            // Generate a random device name
            uint16_t randomNumber = 1000 + (esp_random() % 9000);
            deviceInfo.name = "HOOP_IX_" + String(randomNumber);

            // Generate random UUIDs
            deviceInfo.serviceUUID = generateUUID();
            deviceInfo.characteristicUUID = generateUUID();
            LOG("generated new device info: %s, service [%s], characteristic [%s]",
                deviceInfo.name.c_str(),
                deviceInfo.serviceUUID.c_str(),
                deviceInfo.characteristicUUID.c_str());

            saveDeviceInfo();
        } else {
            ERROR err = deviceHandler.read(deviceInfo);
            BUG_CHECK(err == NONE, "Failed to read device info");
        }

        ServerCallbacks* serverCallbacks = new ServerCallbacks();
        CharacteristicCallbacks* charCallbacks = new CharacteristicCallbacks();
        BLEDevice::init(deviceInfo.name.c_str());
        bleServer = BLEDevice::createServer();
        bleServer->setCallbacks(serverCallbacks);
        bleService = bleServer->createService(deviceInfo.serviceUUID.c_str());
        bleCharacteristic = bleService->createCharacteristic(
            deviceInfo.characteristicUUID.c_str(),
            BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_INDICATE
        );
        bleCharacteristic->setCallbacks(charCallbacks);
    }

    // Starts advertising the BLE service
    void startAdvertising() {
        bleService->start();

        BLEAdvertising* advertising = BLEDevice::getAdvertising();
        advertising->addServiceUUID(deviceInfo.serviceUUID.c_str());
        advertising->setScanResponse(true);
        advertising->setMinPreferred(0x50); // 100ms
        advertising->setMaxPreferred(0xC8); // 250ms
        advertising->setMinInterval(0x320); // 500ms
        advertising->setMaxInterval(0x640); // 1000ms
        BLEDevice::startAdvertising();
        LOG("bluetooth advertising started");
    }

    void stopAdvertising() {
        BLEDevice::getAdvertising()->stop();
    }

    void sendData(String data) {
        bleCharacteristic->setValue(data.c_str());
        bleCharacteristic->indicate();
    }

    bool isConnected() {
        bool connected = bleServer->getConnectedCount() > 0;
        LOG("bluetooth connected: %s", connected ? "yes" : "no");
        return connected;
    }

    void saveDeviceInfo() {
        LOG("saving device info");
        if (haveSavedDeviceInfo()) {
            bool removed = deviceHandler.remove();
            BUG_CHECK(removed, "failed to remove existing device info");
        }
        ERROR err = deviceHandler.write(deviceInfo);
        BUG_CHECK(err == NONE, "failed to save device info");
    }

    bool haveSavedDeviceInfo() {
        bool exists = deviceHandler.exists();
        LOG("saved device info exists: %s", exists ? "yes" : "no");
        return exists;
    }
};

#endif
