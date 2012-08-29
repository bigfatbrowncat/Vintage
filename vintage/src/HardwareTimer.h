class HardwareTimer;

#ifndef HARDWARETIMER_H
#define HARDWARETIMER_H

#include "HardwareDevice.h"
#include "CPU.h"

class HardwareTimer : public HardwareDevice
{
private:
protected:
	virtual bool doAction();
	virtual bool onMessageReceived(const MessageContext& context);

public:
	HardwareTimer(int1* memory, int4 memorySize) :
		HardwareDevice(false, 1, memory, memorySize) {}
	virtual ~HardwareTimer() {}
};

#endif
