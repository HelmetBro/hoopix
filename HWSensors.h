#ifndef HW_SENSORS_H
#define HW_SENSORS_H

#include "Wire.h"
#include <SD.h>
#include <Adafruit_LIS3DH.h>
#include <vl53l4cd_class.h>

#define XSHUT_PIN -1
#define SD_CS_PIN 4

// Adjust this number for the sensitivity of the 'tap' force
// This value strongly depends on the range! for 16G, try 5-10
// for 8G, try 10-20. for 4G try 20-40. for 2G try 40-80
#define TAP_THRESHOLD 80

static VL53L4CD g_tofSensor(&Wire, XSHUT_PIN);
static Adafruit_LIS3DH g_AccelSensor;

bool initializeSD() {
  bool init = SD.begin(SD_CS_PIN);
#if DEBUG
  if (!init)
    Serial.println("Failed to initialize SD card");
  else
    Serial.println("Initialized SD card!");
#endif
  return init;
}

bool initializeToFSensor() {
  Wire.begin();
  g_tofSensor.begin();
  g_tofSensor.VL53L4CD_Off();
  g_tofSensor.InitSensor();
  g_tofSensor.VL53L4CD_SetRangeTiming(200, 0);
  g_tofSensor.VL53L4CD_StartRanging();
  return true;
}

bool initializeAccelSensor() {
  bool init = g_AccelSensor.begin(0x18); // change this to 0x19 for alternative i2c address
#if DEBUG
  if (!init)
    Serial.println("Couldn't start LIS3DH");
  else
    Serial.println("LIS3DH found!");
#endif

  g_AccelSensor.setRange(LIS3DH_RANGE_2_G); // 2, 4, 8 or 16 G!

#if DEBUG
  Serial.print("Range = "); Serial.print(2 << g_AccelSensor.getRange());  
  Serial.println("G");
#endif

  // 0 = turn off click detection & interrupt
  // 1 = single click only interrupt output
  // 2 = double click only interrupt output, detect single click
  // Adjust threshold, higher numbers are less sensitive
  g_AccelSensor.setClick(2, TAP_THRESHOLD);
  return true;
}

#endif