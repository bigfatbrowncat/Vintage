class HardwareDevice;

#ifndef HARDWAREDEVICE_H_
#define HARDWAREDEVICE_H_

#include <pthread.h>

#include "instructions.h"
#include "CPU.h"
#include "SDLScreen.h"
#include "Debugger.h"

#define	TERMINAL_CALL_PRINT									1
#define	TERMINAL_CALL_MOVECURSOR							2

class HardwareDevice
{
private:
	CPU& cpu;
	int port;
	volatile bool turnOffPending;
	volatile bool isTurnedOff;

	pthread_t activity;
	friend void* HardwareDevice_activity_function(void* arg);
protected:
	bool TurnOffPending()
	{
		return turnOffPending;
	}
	virtual void ActivityFunction() = 0;
public:
	CPU& GetCPU() { return cpu; }
	int GetPort() {	return port; }

	void TurnOn();
	void TurnOff();
	virtual void CallHandler(int1* heap, int1* stack, int4 stp) = 0;

	HardwareDevice(CPU& cpu, int port);
	virtual ~HardwareDevice();

};

#endif /* HARDWAREDEVICE_H_ */
