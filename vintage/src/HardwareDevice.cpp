#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "HardwareDevice.h"
#include "MessageContext.h"

void* HardwareDevice_activity_function(void* arg)
{
	((HardwareDevice*)arg)->ActivityFunction();
	((HardwareDevice*)arg)->state = hdsOff;
	return NULL;
}


HardwareDevice::HardwareDevice(int4 portsCount, int1* memory, int4 memorySize) :
	portsCount(portsCount), active(false)
{
	pthread_mutex_init(&controlMutex, NULL);

	this->memory = memory;
	this->memorySize = memorySize;

	devicesConnectedToPorts = new HardwareDevice*[portsCount];
	for (int i = 0; i < portsCount; i++) devicesConnectedToPorts[i] = NULL;

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

	dev1.devicesConnectedToPorts[port1] = &dev2;
	dev2.devicesConnectedToPorts[port2] = &dev1;

	dev1.onOtherDeviceConnected(port1);
	dev2.onOtherDeviceConnected(port2);

	pthread_mutex_unlock(&dev1.controlMutex);
	pthread_mutex_unlock(&dev2.controlMutex);
}

void HardwareDevice::sendMessage()
{
	pthread_mutex_lock(&controlMutex);

	if (devicesConnectedToPorts[activityContext.port] != NULL)
	{
		// Creating "context to receive"
		MessageContext contextToReceive = activityContext;
		contextToReceive.port = devicesConnectedToPorts[activityContext.port]->portIndexOfConnectedDevice(*this);

		devicesConnectedToPorts[activityContext.port]->onMessageReceived(contextToReceive);
	}

	pthread_mutex_unlock(&controlMutex);
}


bool HardwareDevice::onMessageReceived(const MessageContext& context)
{
	int1* stack = &(getMemory()[context.stackStart]);
	int1* heap = &(getMemory()[context.heapStart]);

	int4 command = *((int4*)&stack[context.stackPtr + 0]);
	if (command == HARDWARE_INITIALIZE)
	{
		int1* new_activity_context = ((int1*)&stack[context.stackPtr + 4]);
		activityContext.readFrom(new_activity_context);
		return true;	// Handled
	}
	else if (command == HARDWARE_ACTIVATE)
	{
		active = true;
		return true;	// Handled
	}
	else if (command == HARDWARE_DEACTIVATE)
	{
		active = false;
		return true;	// Handled
	}
	else
	{
		return false;	// Not handled
	}
}
