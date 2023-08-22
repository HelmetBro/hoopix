#ifndef ISTATE_H
#define ISTATE_H

#include "Power.h"
#include "SmartBluetooth.h"
#include "ShotDetector.h"
#include "SmartWifi.h"
#include "Utils.h"

class IState {
public:
    inline static ShotDetector* shotDetector = new ShotDetector(100, 50);
    inline static ShotData* shotData = new ShotData();
    inline static SmartWifi* wifi = new SmartWifi();
    inline static SmartBluetooth* bluetooth = new SmartBluetooth();
    inline static Power* power = new Power();

    IState(){}
    virtual ~IState(){}

    virtual STATE step() = 0;
    virtual STATE getState() = 0;
};

#endif
