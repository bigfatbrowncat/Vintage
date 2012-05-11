#ifndef HARDWAREDEVICE_H_
#define HARDWAREDEVICE_H_

#include <pthread.h>

#include "instructions.h"
#include "SDLConsole.h"
#include "CPU.h"

#define	TERMINAL_CALL_PRINT									1
#define	TERMINAL_CALL_MOVECURSOR							2

class CPU;
class SDLScreen;

class HardwareDevice {
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


class HardwareTimer : public HardwareDevice
{
protected:
	void ActivityFunction();
	void CallHandler(int1* heap, int1* stack, int4 stp) {}
public:
	HardwareTimer(CPU& cpu, int port): HardwareDevice(cpu, port) {}
};


class Console : public HardwareDevice
{
private:
	SDLScreen* window;
protected:
	void ActivityFunction() {}
	void CallHandler(int1* heap, int1* stack, int4 stp);
public:
	Console(CPU& cpu, int port, SDLScreen* window);
};

enum KeyModifiers
{
	KEYMOD_NONE  = 0x0000,
	KEYMOD_LSHIFT= 0x0001,
	KEYMOD_RSHIFT= 0x0002,
	KEYMOD_LCTRL = 0x0004,
	KEYMOD_RCTRL = 0x0008,
	KEYMOD_LALT  = 0x0010,
	KEYMOD_RALT  = 0x0020,
	KEYMOD_LMETA = 0x0040,
	KEYMOD_RMETA = 0x0080,
	KEYMOD_NUM   = 0x0100,
	KEYMOD_CAPS  = 0x0200,
	KEYMOD_MODE  = 0x0400
};

class Keyboard : public HardwareDevice
{
private:
	pthread_mutex_t keyBufferLock;

	int2 bufferLength;

	int4 current, last;
	int1* keyDown;
	int2* modifiers;
	int4* keyCode;
public:
	Keyboard(CPU& cpu, int port, int2 bufferLength);
	~Keyboard();
	void ChangeKeyState(bool key_down, KeyModifiers modifiers, int4 key_code);
	void CallHandler(int1* heap, int1* stack, int4 stp) {}
	void ActivityFunction();
};

#endif /* HARDWAREDEVICE_H_ */
