#ifndef HW_SENSORS_H
#define HW_SENSORS_H

#include "Wire.h"
#include <SD.h>
#include <Adafruit_LIS3DH.h>
#include <vl53l4cd_class.h>

#define XSHUT_PIN -1
#define SD_CS_PIN 5
#define TOF_SDA_PIN 10
#define TOF_SCL_PIN 9

// Adjust this number for the sensitivity of the 'tap' force
// This value strongly depends on the range! for 16G, try 5-10
// for 8G, try 10-20. for 4G try 20-40. for 2G try 40-80
#define TAP_THRESHOLD 80

static VL53L4CD g_tofSensor(&Wire, XSHUT_PIN);

static Adafruit_LIS3DH g_AccelSensor;

void initializeSD() {
    bool init = SD.begin(SD_CS_PIN);
    BUG_CHECK(init, "SD.begin() failed");
}

void initializeToFSensor() {
    bool init = Wire.begin();
    BUG_CHECK(init, "Wire.begin() failed");

    init = g_tofSensor.begin();
    BUG_CHECK(init, "VL53L4CD.begin() failed");

    g_tofSensor.VL53L4CD_Off();

    VL53L4CD_ERROR err = g_tofSensor.InitSensor();
    BUG_CHECK(err == 0, "VL53L4CD.InitSensor() failed");

    err = g_tofSensor.VL53L4CD_SetRangeTiming(200, 0);
    BUG_CHECK(err == 0, "VL53L4CD.SetRangeTiming() failed");

    err = g_tofSensor.VL53L4CD_StartRanging();
    BUG_CHECK(err == 0, "VL53L4CD.StartRanging() failed");

    LOG("VL53L4CD found!");
}

void initializeAccelSensor() {
    bool init = g_AccelSensor.begin(0x18); // change this to 0x19 for alternative i2c address
    BUG_CHECK(init, "LIS3DH.begin() failed");
    g_AccelSensor.setRange(LIS3DH_RANGE_2_G); // 2, 4, 8 or 16 G!

    // 0 = turn off click detection & interrupt
    // 1 = single click only interrupt output
    // 2 = double click only interrupt output, detect single click
    // Adjust threshold, higher numbers are less sensitive
    g_AccelSensor.setClick(2, TAP_THRESHOLD);

    LOG("LIS3DH found!");

// TODO: remove later
#if DEBUG
    Serial.print("Range = "); Serial.print(2 << g_AccelSensor.getRange());
    Serial.println("G");
#endif
}

#endif
