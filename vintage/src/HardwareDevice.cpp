#include <time.h>
#include <unistd.h>

#include "HardwareDevice.h"

void* HardwareDevice_activity_function(void* arg)
{
	((HardwareDevice*)arg)->ActivityFunction();
	return NULL;
}


HardwareDevice::HardwareDevice(CPU& cpu, int port): cpu(cpu), port(port)
{
	isTurnedOff = true;
	cpu.devices[port] = this;
}

HardwareDevice::~HardwareDevice()
{
	if (!isTurnedOff)
		TurnOff();
}

void HardwareDevice::TurnOn()
{
	if (isTurnedOff)
	{
		turnOffPending = false;
		isTurnedOff = false;
		pthread_create(&this->activity, NULL, &HardwareDevice_activity_function, this);
	}
}

void HardwareDevice::TurnOff()
{
	if (!isTurnedOff)
	{
		turnOffPending = true;
		void* value;
		pthread_join(activity, &value);
		isTurnedOff = true;
	}
}
