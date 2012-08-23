#ifndef HARDWAREDEVICE_H_
#define HARDWAREDEVICE_H_

#include <pthread.h>

#include "instructions.h"
#include "SDLScreen.h"
#include "Debugger.h"
#include "CPUContext.h"

#include <map>

using namespace std;

#define	TERMINAL_CALL_PRINT									1
#define	TERMINAL_CALL_MOVECURSOR							2

class HardwareDevice;

struct HardwareDeviceConnection
{
	HardwareDevice* other;
	int othersPort;

	HardwareDeviceConnection() {}
	HardwareDeviceConnection(HardwareDevice* other, int othersPort) :
		other(other), othersPort(othersPort) {}
};

enum HardwareDeviceState { hdsTurningOnPending, hdsOn, hdsTurningOffPending, hdsOff };

class HardwareDevice
{
private:
	pthread_mutex_t controlMutex;
	map<int, HardwareDeviceConnection> connections;
	volatile HardwareDeviceState state;
	pthread_t activity;
	int1* memory;
	int4 memorySize;
	friend void* HardwareDevice_activity_function(void* arg);
protected:
	virtual void ActivityFunction() = 0;
	virtual void onOtherDeviceConnected(int4 port) {}
	virtual void onMessageReceived(int4 port, const CPUContext& context) {}

	void sendMessage(int4 port, const CPUContext& context);
	void broadcastMessage(const CPUContext& context)
	{
		for (map<int, HardwareDeviceConnection>::const_iterator citer = connections.begin(); citer != connections.end(); citer++)
		{
			sendMessage((*citer).first, context);
		}

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

	HardwareDevice(int1* memory, int4 memorySize);
	virtual ~HardwareDevice();

};

#endif /* HARDWAREDEVICE_H_ */
