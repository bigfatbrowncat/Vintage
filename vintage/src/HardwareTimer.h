class HardwareTimer;

#ifndef HARDWARETIMER_H
#define HARDWARETIMER_H

#include "HardwareDevice.h"
#include "CPU.h"

class HardwareTimer : public HardwareDevice
{
private:
	bool active;
	CPUContext activityContext;
protected:
	void ActivityFunction();
	void callHandler(int1* heap, int1* stack, int4 stp) {}
public:
	HardwareTimer(int1* memory, int4 memorySize) :
		HardwareDevice(memory, memorySize),
		active(false)
	{}
	virtual ~HardwareTimer() {}
};

#endif
