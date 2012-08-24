class HardwareTimer;

#ifndef HARDWARETIMER_H
#define HARDWARETIMER_H

#include "HardwareDevice.h"
#include "CPU.h"

class HardwareTimer : public HardwareDevice
{
private:
protected:
	void ActivityFunction();

public:
	HardwareTimer(int1* memory, int4 memorySize) :
		HardwareDevice(memory, memorySize)
	{}
	virtual ~HardwareTimer() {}
};

#endif
