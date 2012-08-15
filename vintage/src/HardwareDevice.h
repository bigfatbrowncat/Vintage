#ifndef HARDWAREDEVICE_H_
#define HARDWAREDEVICE_H_

#include <pthread.h>

#include "instructions.h"
#include "SDLScreen.h"
#include "Debugger.h"

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
	friend void* HardwareDevice_activity_function(void* arg);
protected:
	virtual void ActivityFunction() = 0;
	virtual void onOtherDeviceConnected(int4 port) {}
	virtual void onMessageReceived(int4 port, int1* data, int4 length) {}

	/*map<int, HardwareDeviceConnection>::const_iterator getConnectionsBegin() { return connections.begin(); }
	map<int, HardwareDeviceConnection>::const_iterator getConnectionsEnd() { return connections.end(); }*/

	void sendMessage(int4 port, int1* data, int4 length);
	void broadcastMessage(int1* data, int4 length)
	{
		for (map<int, HardwareDeviceConnection>::const_iterator citer = connections.begin(); citer != connections.end(); citer++)
		{
			sendMessage((*citer).first, data, length);
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

	HardwareDevice();
	virtual ~HardwareDevice();

};

#endif /* HARDWAREDEVICE_H_ */
