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
public:

    static const uint16_t DISTANCE_VARIANCE = 20; // 2 cm variance

    ShotDetector(uint16_t baselineDistance, uint16_t shotDistance)
        : baselineDistance(baselineDistance), shotDistance(shotDistance),
            smartWait(20, SmartWait::MILLIS), state(LOOKING_FOR_SHOT) {
    }

    bool detectLongHoldStep() {
        static uint16_t consistentMeasurements = 0;
        static uint16_t threshold = HOLD_DURATION / smartWait.getInitTime();

        if (!smartWait.timePassed())
            return false;

        VL53L4CD_Result_t results;
        VL53L4CD_ERROR err = g_tofSensor.VL53L4CD_GetResult(&results);
        BUG_CHECK(err == 0, "VL53L4CD_GetResult() failed");

        uint16_t distance = results.distance_mm;
        if (distance >= HOLD_SHORT_DISTANCE && distance <= HOLD_LONG_DISTANCE) {
            consistentMeasurements++;
        } else {
            consistentMeasurements = 0; // reset
        }

        if (consistentMeasurements >= threshold) {
            consistentMeasurements = 0; // reset counter for next detection
            LOG("long hold detected");
            return true;
        }

        LOG("long hold not detected");
        return false;
    }

    bool detectShotHitStep() {
        if (!smartWait.timePassed())
            return false;

        VL53L4CD_Result_t results;
        VL53L4CD_ERROR err = g_tofSensor.VL53L4CD_GetResult(&results);
        BUG_CHECK(err == 0, "VL53L4CD_GetResult() failed");

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
                    LOG("shot detected");
                    return true;
                }
                break;
            default: BUG("invalid shot state");
        }

        LOG("Shot not detected");
        return false;
    }

    bool detectShotMissedStep() {
        uint8_t click = g_AccelSensor.getClick();
        bool missDetected = (click & 0x30) != 0; // 0x30 = 00110000
        if (missDetected)
            LOG("shot missed");
        return missDetected;
    }

private:
    enum DETECTOR_STATE {
        LOOKING_FOR_SHOT,
        AWAITING_RETURN
    };

    uint16_t baselineDistance;
    uint16_t shotDistance;
    SmartWait smartWait;
    DETECTOR_STATE state;

    bool isWithinRange(uint16_t value, uint16_t target, uint16_t variance) {
        return value >= target - variance && value <= target + variance;
    }
};

#endif
