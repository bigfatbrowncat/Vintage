class CPUKeyboardController;

#ifndef CPUKEYBOARDCONTROLLER_H_
#define CPUKEYBOARDCONTROLLER_H_

#include "KeyModifiers.h"
#include "HardwareDevice.h"
#include "KeyboardController.h"

#include <pthread.h>

class CPUKeyboardController : public HardwareDevice, public KeyboardController
{
private:
	pthread_mutex_t keyBufferLock;

	int4 bufferLength;

	volatile int4 current, last;
	bool* keyDown;
	int4* keyCode;
public:
	CPUKeyboardController(CPU& cpu, int port, int2 bufferLength);
	virtual ~CPUKeyboardController();
	virtual void ChangeKeyState(bool key_down, int4 key_code);
	void CallHandler(int1* heap, int1* stack, int4 stp) {}
	void ActivityFunction();
};

#endif
