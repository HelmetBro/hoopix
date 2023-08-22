#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

enum ERROR {
    NONE,
    HW_FAIL,
    SD_INIT_FAIL,
    FILE_OPEN_FAIL,
    FILE_SIZE_MISMATCH,
    SENSOR_FAIL
};

enum STATE {
    NORMAL,
    BLUETOOTH_CONNECT
};

class SmartWait {
public:

    enum TIME {
        MICROS,
        MILLIS
    };

    SmartWait(uint32_t initTime, TIME unit) : initTime(initTime), unit(unit),
        lastTime(0), hasStarted(false) {}

    bool timePassed() {
        uint32_t currentTime = (unit == MILLIS) ? millis() : micros();

        // If timer hasn't started, initialize lastTime and set hasStarted to true
        // This will ensure that the first call to timePassed will start the timer
        if (!hasStarted) {
            lastTime = currentTime;
            hasStarted = true;
            return false;
        }

        if ((currentTime - lastTime) >= initTime) {
            lastTime = currentTime;
            return true;
        }
        return false;
    }

    void reset() {
        // Reset the flag so the timer will start on the next call to timePassed
        hasStarted = false;
    }

    uint32_t getInitTime() {
        return initTime;
    }

private:
    const uint32_t initTime;
    uint32_t lastTime;
    TIME unit;
    bool hasStarted;
};

ERROR checkHardware() {
    return NONE;
}

const char* get_error_description(ERROR err) {
    switch (err) {
        case NONE: return "No Error";
        case HW_FAIL: return "Hardware Failure";
        case SD_INIT_FAIL: return "SD Card Initialization Failure";
        case FILE_OPEN_FAIL: return "File Open Failure";
        case FILE_SIZE_MISMATCH: return "File Size Mismatch";
        case SENSOR_FAIL: return "Sensor Failure";
        default: return "Unknown Error";
    }
}

void DEBUG_DELAY(uint32_t ms) {
#if DEBUG
    delay(ms);
#endif
}

void FAST_LOG(const char* format, ...) {
#if DEBUG
    const size_t BUFFER_SIZE = 128;
    char buffer[BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    Serial.println(buffer);
#endif
}

void LOG(const char* format, ...) {
// #if DEBUG
//     va_list args;
//     va_start(args, format);
//     FAST_LOG(format, args);
//     va_end(args);
// #endif
#if DEBUG
    va_list args;
    va_start(args, format);
    // Determine the required buffer size
    size_t size = vsnprintf(nullptr, 0, format, args) + 1;
    va_end(args);

    char* buffer = new char[size];

    va_start(args, format);
    vsnprintf(buffer, size, format, args);
    va_end(args);

    Serial.println(buffer);
    delete[] buffer;
#endif
}

void BUG_CHECK(ERROR error, const char* message) {
    if (error != NONE) {
        Serial.print(get_error_description(error));
        Serial.print(": ");
        Serial.println(message);
        while (true) {}
    }
}

void BUG_CHECK(bool condition, const char* message) {
    if (!condition) {
        Serial.println(message);
        while (true) {}
    }
}

void BUG(const char* message) {
    Serial.println(message);
    while (true) {}
}

#endif
