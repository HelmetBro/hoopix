#ifndef SD_HANDLER_H
#define SD_HANDLER_H

#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include "Utils.h"

template<typename T>
class SDHandler {
public:
  SDHandler(const std::string& filename)
    : filename_(filename) {}

  // Write data to the file
  ERROR write(const T& data) {
    File file = SD.open(filename_.c_str(), FILE_WRITE);
    if (!file) {
      return FILE_OPEN_FAIL;
    }

    file.write(reinterpret_cast<const uint8_t*>(&data), sizeof(T));
    file.close();
    return HW_OK;
  }

  // Read data from the file
  ERROR read(T& data) {
    File file = SD.open(filename_.c_str(), FILE_READ);
    if (!file) {
      return FILE_OPEN_FAIL;
    }

    if (file.size() != sizeof(T)) {
      file.close();
      return FILE_SIZE_MISMATCH;
    }

    file.read(reinterpret_cast<uint8_t*>(&data), sizeof(T));
    file.close();
    return HW_OK;
  }

  // Check if the file exists
  bool exists() const {
    return SD.exists(filename_.c_str());
  }

  // Delete the file from the SD card
  bool remove() {
    if (exists()) {
      return SD.remove(filename_.c_str());
    }
    return false;
  }

private:
  std::string filename_;
};

#endif
