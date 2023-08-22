#ifndef BLUETOOTH_CONNECT_STATE_H
#define BLUETOOTH_CONNECT_STATE_H

#include "Utils.h"
#include "IState.h"
#include "NormalState.h"

#define TIMEOUT (10 * 1000)

class BluetoothConnectState : public IState {
public:
    BluetoothConnectState() :
        animationStepWait(100, SmartWait::MILLIS),
        timeoutWait(TIMEOUT, SmartWait::MILLIS) {}

    STATE step() override {
        if (timeoutWait.timePassed()) {
            LOG("bluetooth advertising timeout reached");
            bluetooth->stopAdvertising();
            BUG_CHECK(!bluetooth->isConnected(), "unexpected connection on timeout");
            return STATE::NORMAL;
        }

        bluetoothAnimationStep(animationStepWait);

        bluetooth->startAdvertising();
        if (bluetooth->isConnected()) {
            connectedAnimation();
            return STATE::NORMAL;
        }

        // bluetooth not connected, staying in current state
        return getState();
    }

    STATE getState() override {
        return STATE::BLUETOOTH_CONNECT;
    }

private:
    SmartWait animationStepWait;
    SmartWait timeoutWait;

    // non-blocking
    void bluetoothAnimationStep(SmartWait& wait) {
        if (!wait.timePassed())
            return;
        LOG("bluetooth animation step");
    }

    // blocking
    void connectedAnimation() {
        LOG("connected animation");
    }
};

#endif
