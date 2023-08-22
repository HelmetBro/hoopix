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

    ERROR write(const T& data) {
        LOG("Writing to file: %s", filename_.c_str());
        File file = SD.open(filename_.c_str(), FILE_WRITE);
        if (!file) {
            ERROR err = FILE_OPEN_FAIL;
            BUG_CHECK(err, "File [%s] open failed for writing", filename_.c_str());
            return err;
        }

        file.write(reinterpret_cast<const uint8_t*>(&data), sizeof(T));
        file.close();
        LOG("Write successful");
        return NONE;
    }

    // Read data from the file
    ERROR read(T& data) {
        LOG("Reading from file: %s", filename_.c_str());
        File file = SD.open(filename_.c_str(), FILE_READ);
        if (!file) {
            ERROR err = FILE_OPEN_FAIL;
            BUG_CHECK(err, "File [%s] open failed for reading", filename_.c_str());
            return err;
        }

        if (file.size() != sizeof(T)) {
            file.close();
            ERROR err = FILE_SIZE_MISMATCH;
            BUG_CHECK(err, "File [%s] size mismatch", filename_.c_str());
            return err;
        }

        file.read(reinterpret_cast<uint8_t*>(&data), sizeof(T));
        file.close();
        LOG("Read successful");
        return NONE;
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
