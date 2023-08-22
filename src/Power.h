#ifndef POWER_H
#define POWER_H

#include "SDHandler.h"
#include "ShotDetector.h"
#include <esp_sleep.h>

#define IDLE_TIMEOUT_MS 60000 // 1 minute in milliseconds
#define WAKE_UP_PIN GPIO_NUM_13
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

    PowerState wakeDeviceProcedure(ShotData* data) {
        // Waking up is done automatically by the ESP32 when the wake up condition is met
        // This is additional processing that happens when the device is woken up
        if (!lastStateHandler.exists() && !shotDataHandler.exists()) {
            LOG("first boot");
            powerState = IS_FIRST_BOOT;
            lastStateHandler.write(powerState);
            shotDataHandler.write(*data);
        } else if (lastStateHandler.exists() && shotDataHandler.exists()) {
            LOG("waking up from sleep");
            lastStateHandler.read(powerState);
            shotDataHandler.read(*data);
        } else {
            BUG("invalid wake-up state");
        }

        PowerState lastState = powerState;
        powerState = WAS_AWAKE;
        return lastState;
    }

    void sleepDevice(ShotData* data) {
        // Define which pin will wake up the ESP32
        esp_err_t esp_err = esp_sleep_enable_ext0_wakeup(WAKE_UP_PIN, 1); // 1 = High, 0 = Low
        BUG_CHECK(esp_err == ESP_OK, "esp_sleep_enable_ext0_wakeup() failed");

        // Set device into hibernation
        esp_err = esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
        BUG_CHECK(esp_err == ESP_OK, "esp_sleep_pd_config() PERIPH failed");

        esp_err = esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
        BUG_CHECK(esp_err == ESP_OK, "esp_sleep_pd_config() SLOW_MEM failed");

        esp_err = esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
        BUG_CHECK(esp_err == ESP_OK, "esp_sleep_pd_config() FAST_MEM failed");

        // Save the current shot data to SD before sleeping
        powerState = WAS_ASLEEP;

        ERROR err = shotDataHandler.write(*data);
        BUG_CHECK(err == NONE, "shotDataHandler.write() failed");

        err = lastStateHandler.write(powerState);
        BUG_CHECK(err == NONE, "lastStateHandler.write() failed");

        LOG("going to sleep. goodnight! <3");
        esp_deep_sleep_start();
    }

    // blocking
    void batteryStartupAnimation(){
        DEBUG_DELAY(250);
        LOG("battery startup animation");
    }

    // blocking
    void awakenAnimation(){
        DEBUG_DELAY(250);
        LOG("awaken animation");
    }

private:
    PowerState powerState;
    SDHandler<ShotData> shotDataHandler;
    SDHandler<PowerState> lastStateHandler;
};

#endif // POWER_H
