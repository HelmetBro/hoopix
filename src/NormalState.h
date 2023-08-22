#ifndef NORMAL_STATE_H
#define NORMAL_STATE_H

#include "Utils.h"
#include "IState.h"
#include "BluetoothConnectState.h"

#define WIFI_MAX_ATTEMPTS 3
#define WIFI_TIMEOUT (10 * 1000)

class NormalState : public IState {
public:
    NormalState() : animationStepWait(1000, SmartWait::MILLIS),
        lowPowerWait(30 * 1000, SmartWait::MILLIS),
        wifiTimeoutWait(WIFI_TIMEOUT, SmartWait::MILLIS) {
        this->shotData->shotsHit = 0;
        this->shotData->shotsMissed = 0;
    }

    // non-blocking
    void normalAnimationStep(SmartWait& wait) {
        if (!wait.timePassed())
            return;
        LOG("normal animation step");
    }

    STATE step() override {
        if(!wifi->maxAttemptsReached() && !wifi->isConnected() &&
            wifi->haveSavedWifi() && !wifiTimeoutWait.timePassed()) {
            wifi->connect();
        }

        normalAnimationStep(animationStepWait);

        bool shotHit = shotDetector->detectShotHitStep();
        if (shotHit)
            shotData->shotsHit++;

        bool shotMissed = shotDetector->detectShotMissedStep();
        if (shotDetector->detectShotMissedStep())
            shotData->shotsMissed++;

        bool longHold = shotDetector->detectLongHoldStep();
        if (longHold)
            return STATE::BLUETOOTH_CONNECT;

        if (switchToLowPower(shotHit, shotMissed)) {
            LOG("low power wait time passed, switching to low power mode");
            power->sleepDevice(shotData);
        }

        return getState();
    }

    STATE getState() override {
        return STATE::NORMAL;
    }

private:
    SmartWait animationStepWait;
    SmartWait lowPowerWait;
    SmartWait wifiTimeoutWait;

    bool switchToLowPower(bool shotHit, bool shotMissed) {
        if (shotHit || shotMissed) {
            LOG("shot/miss detected, resetting low power wait");
            lowPowerWait.reset();
        }
        return lowPowerWait.timePassed();
    }
};

#endif
