class HardwareTimer;

#ifndef HARDWARETIMER_H
#define HARDWARETIMER_H

#include "HardwareDevice.h"
#include "CPU.h"

class HardwareTimer : public HardwareDevice
{
protected:
	void ActivityFunction();
	void CallHandler(int1* heap, int1* stack, int4 stp) {}
public:
	HardwareTimer(CPU& cpu, int port): HardwareDevice(cpu, port) {}
	virtual ~HardwareTimer() {}
};

#endif
