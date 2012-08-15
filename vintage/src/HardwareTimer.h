class HardwareTimer;

#ifndef HARDWARETIMER_H
#define HARDWARETIMER_H

#include "HardwareDevice.h"
#include "CPU.h"

class HardwareTimer : public HardwareDevice
{
protected:
	void ActivityFunction();
	void callHandler(int1* heap, int1* stack, int4 stp) {}
public:
	HardwareTimer() {}
	virtual ~HardwareTimer() {}
};

#endif
