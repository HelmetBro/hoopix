#ifndef BLUETOOTH_CONNECT_STATE_H
#define BLUETOOTH_CONNECT_STATE_H

#include "Utils.h"
#include "IState.h"
#include "NormalState.h"

#define TIMEOUT (10 * 1000)

class BluetoothConnectState : public IState {
private:
  SmartWait animationStepWait;
  SmartWait timeoutWait;

  // non-blocking
  void bluetoothAnimationStep(SmartWait& wait) {
    if (!wait.timePassed())
      return;

#if DEBUG
    Serial.print("#");
#endif
  }

    // blocking
    void connectedAnimation() {
#if DEBUG
      Serial.print("connected!");
      delay(2000);
#endif
    }

public:
  BluetoothConnectState() : 
    animationStepWait(100, SmartWait::MILLIS), 
    timeoutWait(TIMEOUT, SmartWait::MILLIS) {}

  STATE step() override {
    if(timeoutWait.timePassed())
      return STATE::NORMAL;

    bluetoothAnimationStep(animationStepWait);

    bluetooth->connect();

    if (bluetooth->isConnected()) {
      connectedAnimation();
      return STATE::NORMAL;
    }

    return getState();
 }

  STATE getState() override {
     return STATE::BLUETOOTH_CONNECT;
  }
};

#endif
