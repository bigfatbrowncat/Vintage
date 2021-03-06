#ifndef HARDWAREDEVICE_H_
#define HARDWAREDEVICE_H_

#include <pthread.h>

#include "instructions.h"
#include "SDLScreen.h"
#include "Debugger.h"
#include "MessageContext.h"

#include <list>

using namespace std;

#define	HARDWARE_INITIALIZE									0

#define	HARDWARE_SUCCEEDED									64 + 0
#define	HARDWARE_FAILED										64 + 1

#define HARDWARE_CUSTOM										256

class HardwareDevice;

enum HardwareDeviceState { hdsTurningOnPending, hdsOn, hdsTurningOffPending, hdsOff };
enum MessageHandlingResult { mhsNotHandled, mhsSuccessful, mhsUnsuccessful };

class HardwareDevice
{
private:
	pthread_mutex_t controlMutex;
	bool* inputPortIsWaiting;
	MessageContext* portInWaitingContext;
	volatile bool someInputPortIsWaiting;

	HardwareDevice** devicesConnectedToPorts;

	volatile HardwareDeviceState state;
	pthread_t activity;
	int1* memory;
	int4 memorySize;

	friend void* HardwareDevice_activity_function(void* arg);

	void activityFunction();
	void receiveMessage(const MessageContext& context);
protected:
	list<MessageContext> contextStack;

	MessageContext activityContext;
	int portsCount;

	virtual void onOtherDeviceConnected(int4 port) {}

	virtual MessageHandlingResult handleMessage();
	virtual void doCycle(MessageHandlingResult handlingResult);

	void sendMessage();

	int portIndexOfConnectedDevice(const HardwareDevice& dev)
	{
		for (int i = 0; i <= portsCount; i++)
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

	int1* getMemory()
	{
		return memory;
	}
	int4 getMemorySize()
	{
		return memorySize;
	}

	HardwareDevice(bool initiallyActive, int4 portsCount, int1* memory, int4 memorySize);
	virtual ~HardwareDevice();

};

#endif /* HARDWAREDEVICE_H_ */
