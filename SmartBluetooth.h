#ifndef SMART_BLUETOOTH_H
#define SMART_BLUETOOTH_H

#include "SDHandler.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define BLUETOOTH_DEVICE_FILE "/btDevice.bin"

// https://www.uuidgenerator.net/
#define SERVICE_UUID        "d3e10063-a8b8-4e10-ac85-1dd6e1473301"
#define CHARACTERISTIC_UUID "5909ab86-d4bc-490f-a97b-fb1b1cb3c875"

struct DeviceInfo {
  String serviceUUID;
  String characteristicUUID;
  String name;
  std::vector<String> pastDevices;
};

class ServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    pServer->getAdvertising()->stop();
  }

  void onDisconnect(BLEServer* pServer) {
    pServer->startAdvertising();
  }
};

class CharacteristicCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (value.length() > 0) {
            Serial.println("Received Value:");
            for (int i = 0; i < value.length(); i++) {
                Serial.print(value[i]);
            }
            Serial.println();
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
  }

  void init() {
    if (!haveSavedDeviceInfo()) {
      // Generate a random device name
      uint16_t randomNumber = 1000 + (esp_random() % 9000);
      deviceInfo.name = "HOOP_IX_" + String(randomNumber);

      // Generate random UUIDs
      deviceInfo.serviceUUID = generateUUID();
      deviceInfo.characteristicUUID = generateUUID();

      saveDeviceInfo();
    } else {
      deviceHandler.read(deviceInfo);
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
    advertising->addServiceUUID(SERVICE_UUID);
    advertising->setScanResponse(true);
    advertising->setMinPreferred(0x50);  // 100ms
    advertising->setMaxPreferred(0xC8);  // 250ms
    advertising->setMinInterval(0x320);  // 500ms
    advertising->setMaxInterval(0x640);  // 1000ms
    BLEDevice::startAdvertising();
  }

  void stopAdvertising() {
    BLEDevice::getAdvertising()->stop();
  }

  void sendData(String data) {
    bleCharacteristic->setValue(data.c_str());
    bleCharacteristic->indicate();
  }

  bool isConnected() {
    return bleServer->getConnectedCount() > 0;
  }

  void saveDeviceInfo() {
    if (haveSavedDeviceInfo()) {
      deviceHandler.remove();
    }
    deviceHandler.write(deviceInfo);
  }

  bool haveSavedDeviceInfo() {
    return deviceHandler.exists();
  }
};

#endif
