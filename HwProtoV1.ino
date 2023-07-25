/*
- do any multithreading later
- any "animation" is considered to have sound too
- ultimately, i want to save data on the cloud
*/

#define DEBUG 1

#include "NormalState.h"
#include "BluetoothConnectState.h"

static IState* g_state;
static STATE oldState = STATE::NORMAL;

// happens when you put batteries in or wakes up from sleep
void setup() {
#if DEBUG
  Serial.begin(115200);
  while (!Serial) { ; /*wait for serial port to connect*/ }
#endif

  ERROR hwOk = checkHardware();
  if (!hwOk) {
    terminate(hwOk);
  }

  initializeToFSensor();
  initializeAccelSensor();
  initializeSD();

  g_state = new NormalState();

  PowerState lastPowerState = g_state->power->wakeDeviceProcedure(g_state->shotData);
  switch (lastPowerState) {
    case IS_FIRST_BOOT:
      g_state->power->batteryStartupAnimation();
      break;
    case WAS_ASLEEP:
      g_state->power->awakenAnimation();
      break;
    case WAS_AWAKE:
      g_state->power->awakenAnimation();
      break;
    default: break; // TODO: invalid state
  }
}

void loop() {
  STATE newState = g_state->step();

  if (newState != oldState) {
    oldState = newState;
    delete g_state;
    switch(oldState) {
        case STATE::NORMAL: 
          g_state = new NormalState(); 
          break;
        case STATE::BLUETOOTH_CONNECT:
          g_state = new BluetoothConnectState();
          break;
        default: break;
          //error
    }
  }
  // This is intended to slow down the progam a little. Need to see if this is necessary.
  // Could possible save on power. Also need to look into lowering the clock speed to save
  // more power.
  delay(1);
}
