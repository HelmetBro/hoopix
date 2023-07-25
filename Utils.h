#ifndef UTILS_H
#define UTILS_H

enum ERROR {
  HW_OK,
  HW_FAIL,
  SD_NOT_INIT,
  FILE_OPEN_FAIL,
  FILE_SIZE_MISMATCH,
  SENSOR_FAIL
};

enum STATE {
  NORMAL,
  BLUETOOTH_CONNECT
};

struct SmartWait {
  enum TIME {
    MICROS,
    MILLIS
  };

  const uint32_t initTime;
  uint32_t lastTime;
  TIME unit;
  bool hasStarted; // New flag to indicate if the timer has started

  SmartWait(uint32_t initTime, TIME unit) 
    : initTime(initTime), 
      unit(unit), 
      lastTime(0), 
      hasStarted(false) {}  // Initialize hasStarted to false
  
  bool timePassed() {
    uint32_t currentTime = (unit == MILLIS) ? millis() : micros();
    
    // If timer hasn't started, initialize lastTime and set hasStarted to true
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
    hasStarted = false;  // Reset the flag so the timer will start on the next call to timePassed
  }
};

ERROR checkHardware() {
    // Initialize or test hardware components here.
    // If a hardware check fails, return an appropriate error code.

    // For example:
    // if (!testSensor()) {
    //     return SENSOR_FAIL;
    // }

    // If all checks pass, return HW_OK.
    return HW_OK;
}

void terminate(ERROR error) {
  // write error to storage
}

#endif