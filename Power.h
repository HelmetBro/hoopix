#ifndef POWER_H
#define POWER_H

#include "SDHandler.h"
#include "ShotDetector.h"
#include <esp_sleep.h>

#define IDLE_TIMEOUT_MS 60000 // 1 minute in milliseconds
#define WAKE_UP_PIN GPIO_NUM_33
#define SHOT_DATA_FILE "/shotData.bin"
#define LAST_STATE_FILE "/lastState.bin"

enum PowerState {
  WAS_AWAKE,
  WAS_ASLEEP,
  IS_FIRST_BOOT
};

// note that when waking up from in lower power mode, the entire program gets reset
class Power {
  public:
    Power() : powerState(WAS_AWAKE), shotDataHandler(SHOT_DATA_FILE), lastStateHandler(LAST_STATE_FILE) {}

    void sleepDevice(ShotData* data) {
      // Define which pin will wake up the ESP32
      esp_sleep_enable_ext0_wakeup(WAKE_UP_PIN, 1); // 1 = High, 0 = Low

      // Set device into hibernation
      esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
      esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
      esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);

      // Save the current shot data to SD before sleeping
      powerState = WAS_ASLEEP;
      shotDataHandler.write(*data);
      lastStateHandler.write(powerState);

      esp_deep_sleep_start();
    }

    PowerState wakeDeviceProcedure(ShotData* data) {
      // Waking up is done automatically by the ESP32 when the wake up condition is met
      // This is additional processing that happens when the device is woken up
      
      if (!lastStateHandler.exists() && !shotDataHandler.exists()) {
        powerState = IS_FIRST_BOOT;
        lastStateHandler.write(powerState);
        shotDataHandler.write(*data);
      } else if (lastStateHandler.exists() && shotDataHandler.exists()) {
        lastStateHandler.read(powerState);
        shotDataHandler.read(*data);
      } else {
        // TODO: invalid state, handle later
      }

      PowerState lastState = powerState;
      powerState = WAS_AWAKE;
      return lastState;
    }

    // blocking
    void batteryStartupAnimation(){
      delay(250);
#if DEBUG
      Serial.print("Battery startup animation");
#endif
    }

    // blocking
    void awakenAnimation(){
      delay(250);
#if DEBUG
      Serial.print("Awaken animation");
#endif
    }

  private:
    PowerState powerState;
    SDHandler<ShotData> shotDataHandler;
    SDHandler<PowerState> lastStateHandler;
};

#endif // POWER_H
