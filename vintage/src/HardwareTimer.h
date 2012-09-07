class HardwareTimer;

#ifndef HARDWARETIMER_H
#define HARDWARETIMER_H

#include "HardwareDevice.h"
#include "CPU.h"

#define HARDWARE_TIMER_REPORT_TIME		HARDWARE_CUSTOM + 0

class HardwareTimer : public HardwareDevice
{
private:
protected:
	virtual bool handleMessage();
	virtual bool doCycle();

public:
	HardwareTimer(int1* memory, int4 memorySize) :
		HardwareDevice(false, 1, memory, memorySize) {}
	virtual ~HardwareTimer() {}
};

#endif
