#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "HardwareDevice.h"

void* HardwareDevice_activity_function(void* arg)
{
	((HardwareDevice*)arg)->ActivityFunction();
	((HardwareDevice*)arg)->state = hdsOff;
	return NULL;
}


HardwareDevice::HardwareDevice(int1* memory, int4 memorySize)
{
	pthread_mutex_init(&controlMutex, NULL);

	this->memory = memory;
	this->memorySize = memorySize;
	// The initial state os "Off"
	state = hdsOff;
}

HardwareDevice::~HardwareDevice()
{
	// Turning off before destruction
	turnOff();

	pthread_mutex_destroy(&controlMutex);
}

bool HardwareDevice::turnOn()
{
	// Checking if the device is turned off
	if (state == hdsOff)
	{
		pthread_mutex_lock(&controlMutex);

		state = hdsTurningOnPending;
		// Turning it on
		pthread_create(&this->activity, NULL, &HardwareDevice_activity_function, this);
		state = hdsOn;

		pthread_mutex_unlock(&controlMutex);

		return true;
	}
	else
	{
		// Returning false otherwise
		return false;
	}
}

bool HardwareDevice::turnOff()
{
	// Checking if the device is turned on
	if (state == hdsOn)
	{
		pthread_mutex_lock(&controlMutex);

		state = hdsTurningOffPending;
		void* value;
		pthread_join(activity, &value);
		state = hdsOff;

		pthread_mutex_unlock(&controlMutex);

		return true;
	}
	else
	{
		// Returning false otherwise
		return false;
	}

}

void HardwareDevice::connectDevices(HardwareDevice& dev1, int port1, HardwareDevice& dev2, int port2)
{
	pthread_mutex_lock(&dev1.controlMutex);
	pthread_mutex_lock(&dev2.controlMutex);

	dev1.connections.insert(pair<int, HardwareDeviceConnection>(port1, HardwareDeviceConnection(&dev2, port2)));
	dev2.connections.insert(pair<int, HardwareDeviceConnection>(port2, HardwareDeviceConnection(&dev1, port1)));
	dev1.onOtherDeviceConnected(port1);
	dev2.onOtherDeviceConnected(port2);

	pthread_mutex_unlock(&dev1.controlMutex);
	pthread_mutex_unlock(&dev2.controlMutex);
}

void HardwareDevice::sendMessage(int4 port, const CPUContext& context)
{
	pthread_mutex_lock(&controlMutex);

	if (connections.find(port) != connections.end())
	{
		connections[port].other->onMessageReceived(connections[port].othersPort, context);
	}

	pthread_mutex_unlock(&controlMutex);
}
