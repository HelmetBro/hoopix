#ifndef SMART_BLUETOOTH_H
#define SMART_BLUETOOTH_H

#include "SDHandler.h"
#include <BluetoothSerial.h>
#include <thread>

#define BLUETOOTH_DEVICE_FILE "/btDevice.bin"

struct DeviceName {
  String name;
};

class SmartBluetooth {
private:
  BluetoothSerial btSerial;
  SmartWait smartWait;
  SDHandler<DeviceName> deviceHandler;
  DeviceName deviceInfo;

public:
  SmartBluetooth() : smartWait(500, SmartWait::MILLIS), deviceHandler(BLUETOOTH_DEVICE_FILE) {}

  // Initializes the Bluetooth Serial library with a given device name.
  // This is the name other Bluetooth devices will see when discovering this device.
  void begin(const char* name) {
    deviceInfo.name = name;
    btSerial.begin(name);
  }

  // Checks whether any client is currently connected to the Bluetooth Serial server.
  // Returns true if a client is connected, and false otherwise.
  bool isConnected() {
    return btSerial.hasClient();
  }

  // Checks how many bytes of data are currently available to read.
  // Returns the number of bytes in the incoming buffer.
  int available() {
    return btSerial.available();
  }

  // Reads the next byte of incoming serial data.
  // Returns the byte as an integer.
  int read() {
    return btSerial.read();
  }

  // Writes a single byte of data over the Bluetooth serial connection.
  // Returns the number of bytes written (should always be 1 in this case).
  size_t write(uint8_t data) {
    return btSerial.write(data);
  }

  // Disconnects any current client by ending the Bluetooth serial connection
  // and then reinitializing it.
  void disconnect() {
    btSerial.end();
    btSerial.begin();
  }
  
  // Non-blocking connect
  bool connect() {
    static bool isConnecting = false;
    static std::thread connectThread;  // Declare a thread object

    if (!isConnecting) {
      // Spawn a thread to handle the connection
      connectThread = std::thread([this] {
        btSerial.connect(deviceInfo.name);
      });
      isConnecting = true;
    }

    if (smartWait.timePassed() && isConnected()) {
      connectThread.join();  // Ensure the thread finishes cleanly
      isConnecting = false;
      return true;
    }

    return false;
  }

  // Save the device name to the SD card
  void saveDeviceName() {
    if (haveSavedDeviceName()) {
      deviceHandler.remove();
    }
    deviceHandler.write(deviceInfo);
  }

  // Check if the device name is saved on the SD card
  bool haveSavedDeviceName() {
    return deviceHandler.exists();
  }
};

#endif
