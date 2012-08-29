#ifndef HARDWAREDEVICE_H_
#define HARDWAREDEVICE_H_

#include <pthread.h>

#include "instructions.h"
#include "SDLScreen.h"
#include "Debugger.h"
#include "MessageContext.h"

#include <map>

using namespace std;

#define	HARDWARE_INITIALIZE									0
#define	HARDWARE_ACTIVATE									1
#define	HARDWARE_DEACTIVATE									2

#define HARDWARE_CUSTOM										256

class HardwareDevice;

enum HardwareDeviceState { hdsTurningOnPending, hdsOn, hdsTurningOffPending, hdsOff };

class HardwareDevice
{
private:
	pthread_mutex_t controlMutex;

	HardwareDevice** devicesConnectedToPorts;

	volatile HardwareDeviceState state;
	pthread_t activity;
	int1* memory;
	int4 memorySize;

	bool active;

	friend void* HardwareDevice_activity_function(void* arg);
protected:
	MessageContext activityContext;
	int portsCount;

	virtual void ActivityFunction() = 0;
	virtual void onOtherDeviceConnected(int4 port) {}
	virtual bool onMessageReceived(const MessageContext& context);

	void sendMessage();

	int portIndexOfConnectedDevice(const HardwareDevice& dev)
	{
		for (int i = 0; i < portsCount; i++)
		{
			if (devicesConnectedToPorts[i] == &dev) return i;
		}
		return -1;
	}

	void issueTurningOff()
	{
		if (state == hdsOn)
		{
			pthread_mutex_lock(&controlMutex);
			state = hdsTurningOffPending;
			pthread_mutex_unlock(&controlMutex);
		}
	}
public:
	HardwareDeviceState getState() { return state; }
	static void connectDevices(HardwareDevice& dev1, int port1, HardwareDevice& dev2, int port2);

	bool turnOn();
	bool turnOff();

	bool isActive()
	{
		return active;
	}
	int1* getMemory()
	{
		return memory;
	}
	int4 getMemorySize()
	{
		return memorySize;
	}

	HardwareDevice(int4 portsCount, int1* memory, int4 memorySize);
	virtual ~HardwareDevice();

};

#endif /* HARDWAREDEVICE_H_ */
