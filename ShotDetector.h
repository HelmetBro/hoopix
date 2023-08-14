#ifndef SHOT_DETECTOR_H
#define SHOT_DETECTOR_H

#include "HWSensors.h"

#define HOLD_DURATION 3000
// In millimeters
#define HOLD_SHORT_DISTANCE 20
#define HOLD_LONG_DISTANCE 200

struct ShotData {
  uint32_t shotsHit = 0;
  uint32_t shotsMissed = 0;
};

struct ShotDetector {
  enum DETECTOR_STATE {
    LOOKING_FOR_SHOT,
    AWAITING_RETURN
  };

  static const uint16_t DISTANCE_VARIANCE = 20; // 2 cm variance

  uint16_t baselineDistance;
  uint16_t shotDistance;
  SmartWait smartWait;
  DETECTOR_STATE state;

  ShotDetector(uint16_t baselineDistance, uint16_t shotDistance)
    : baselineDistance(baselineDistance), shotDistance(shotDistance),
      smartWait(20, SmartWait::MILLIS), state(LOOKING_FOR_SHOT) {
  }

  bool detectLongHoldStep() {
    static uint16_t consistentMeasurements = 0;
    static uint16_t threshold = HOLD_DURATION / smartWait.initTime;

    if (!smartWait.timePassed())
      return false;

    VL53L4CD_Result_t results;
    g_tofSensor.VL53L4CD_GetResult(&results);
    uint16_t distance = results.distance_mm;

    if (distance >= HOLD_SHORT_DISTANCE && distance <= HOLD_LONG_DISTANCE) {
      consistentMeasurements++;
    } else {
      consistentMeasurements = 0; // reset
    }

    if (consistentMeasurements >= threshold) {
      consistentMeasurements = 0; // reset counter for next detection
      return true;
    }

    return false;
  }

  bool detectShotHitStep() {
    if (!smartWait.timePassed())
      return false;

    VL53L4CD_Result_t results;
    g_tofSensor.VL53L4CD_GetResult(&results);
    uint16_t distance = results.distance_mm;

    switch (state) {
      case LOOKING_FOR_SHOT:
        if (distance < shotDistance) {
          state = AWAITING_RETURN;
        }
        break;

      case AWAITING_RETURN:
        if (isWithinRange(distance, baselineDistance, DISTANCE_VARIANCE)) {
          state = LOOKING_FOR_SHOT; // reset the state
          return true; // shot detected
        }
        break;
    }
    return false; // no shot detected
  }

  bool detectShotMissedStep() {
    uint8_t click = g_AccelSensor.getClick();
    return (click & 0x30) != 0; // returns true if a miss is detected, false otherwise
  }

  bool isWithinRange(uint16_t value, uint16_t target, uint16_t variance) {
    return value >= target - variance && value <= target + variance;
  }
};

#endif
