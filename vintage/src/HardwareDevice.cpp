#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "HardwareDevice.h"
#include "MessageContext.h"

void* HardwareDevice_activity_function(void* arg)
{
	((HardwareDevice*)arg)->activityFunction();
	((HardwareDevice*)arg)->state = hdsOff;
	return NULL;
}

void HardwareDevice::activityFunction()
{
	bool portHandlingJustFinished = false;

	while (getState() == hdsOn)
	{
		if (!portHandlingJustFinished)
		{
			// Checking ports input
			pthread_mutex_lock(&controlMutex);
			if (someInputPortIsWaiting)
			{
				// Clearing our flag (doing this by default, we will check it later)
				someInputPortIsWaiting = false;

				int inputPortsWaitingCount = 0;
				int portToHandle = -1;

				// Here we are checking if any port is waiting.
				// We don't consider ports, which numbers are greater or equal
				// than the currently handling one.
				// This means that the least the index of the port is, the most priority does it have.
				// (The only exception is if the currently handling one has index 0
				// -- in that case it's priority is the lowest)

				// Example: if we are handling port #3 and port #4 is waiting,
				//          the handler for the port #4 will not be executed
				//          until port #3 handling is done.

				// And we start handling for the highest priority
				// (i.e. the least index) port
				for (int i = 0; i <= portsCount; i++)
				{
					// TODO: There is a great problem here...

					if (inputPortIsWaiting[i])
					{
						inputPortsWaitingCount++;
						if (portToHandle == -1 && (contextStack.size() == 0 || i < contextStack.back().port))
						{
							// If it is the first port we found
							// and it's priority is greater than the currently handling ones,
							// saving it for handling NOW
							portToHandle = i;
						}
						else
						{
							// In all other cases it should be handled LATER,
							// so we have a flag to set
							someInputPortIsWaiting = true;

							// Nothing to search more
							break;
						}
					}
				}

				// Calling the handler
				if (portToHandle >= 0)		// it's better to use a flag here
				{
					inputPortIsWaiting[portToHandle] = false;
					// If it was the last port to handle, clearing the flag
					if (inputPortsWaitingCount == 1) someInputPortIsWaiting = false;

					// Adding the context of the port we are handling to the contexts stack
					contextStack.push_back(portInWaitingContext[portToHandle]);

					/*int1* stack = &(getMemory()[contextStack.back().stackStart]);
					int4 command = *((int4*)&stack[contextStack.back().stackPtr + 0]);*/
					handleMessage();
				}
			}
			pthread_mutex_unlock(&controlMutex);
		}
		else
		{
			portHandlingJustFinished = false;
		}

		if (contextStack.size() > 0)
		{
			doCycle();
		}
		else
		{
			usleep(100);
		}
	}
}

HardwareDevice::HardwareDevice(bool initiallyActive, int4 portsCount, int1* memory, int4 memorySize) :
	active(initiallyActive), portsCount(portsCount)
{
	pthread_mutex_init(&controlMutex, NULL);

	inputPortIsWaiting = new bool[portsCount + 1];
	for (int i = 0; i <= portsCount; i++)
	{
		inputPortIsWaiting[i] = false;
	}
	portInWaitingContext = new MessageContext[portsCount + 1];
	someInputPortIsWaiting = false;

	this->memory = memory;
	this->memorySize = memorySize;

	devicesConnectedToPorts = new HardwareDevice*[portsCount + 1];
	devicesConnectedToPorts[0] = this;	// self-connection at zero port
	for (int i = 1; i <= portsCount; i++)
	{
		devicesConnectedToPorts[i] = NULL;
	}

	// The initial state os "Off"
	state = hdsOff;
}

HardwareDevice::~HardwareDevice()
{
	// Turning off before destruction
	turnOff();

	delete[] inputPortIsWaiting;
	delete[] portInWaitingContext;

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

		devicesConnectedToPorts[activityContext.port]->receiveMessage(contextToReceive);
	}

	pthread_mutex_unlock(&controlMutex);
}

void HardwareDevice::receiveMessage(const MessageContext& context)
{
	pthread_mutex_lock(&controlMutex);
	someInputPortIsWaiting = true;
	inputPortIsWaiting[context.port] = true;
	portInWaitingContext[context.port] = context;
	pthread_mutex_unlock(&controlMutex);
}

bool HardwareDevice::handleMessage()
{
	int1* stack = &(getMemory()[contextStack.back().stackStart]);
	int1* heap = &(getMemory()[contextStack.back().heapStart]);

	int4 command = *((int4*)&stack[contextStack.back().stackPtr + 0]);
	if (command == HARDWARE_INITIALIZE)
	{
		int1* new_activity_context = ((int1*)&stack[contextStack.back().stackPtr + 4]);
		activityContext.readFrom(new_activity_context);
		return true;	// Handled
	}
	else if (command == HARDWARE_ACTIVATE)
	{
		active = true;
		contextStack.back().stackPtr -= 4;	// preparing the place for status code
		return true;	// Handled
	}
	else if (command == HARDWARE_DEACTIVATE)
	{
		active = false;
		contextStack.back().stackPtr += 4;	// preparing the place for status code
		return true;	// Handled
	}
	else
	{
		return false;	// Not handled
	}
}

bool HardwareDevice::doCycle()
{
	// Reporting success for any operation
	// TODO: we should check if the operation is really succeeded
	int1* stack = &(getMemory()[activityContext.stackStart]);
	int4* p_result = ((int4*)&stack[activityContext.stackPtr + 0]);
	*p_result = HARDWARE_SUCCEEDED;
	sendMessage();

	contextStack.pop_back();
}
