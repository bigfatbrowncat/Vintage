#ifndef CPUKEYBOARDCONTROLLER_H_
#define CPUKEYBOARDCONTROLLER_H_

#include "KeyModifiers.h"
#include "HardwareDevice.h"
#include "KeyboardController.h"
#include "CPUContext.h"

#include <pthread.h>

class CPUKeyboardController : public HardwareDevice, public KeyboardController
{
private:
	pthread_mutex_t keyBufferLock;

	CPUContext activityContext;
	bool active;

	int4 bufferLength;

	volatile int4 current, last;
	bool* keyDown;
	int4* keyCode;
public:
	CPUKeyboardController(int4 bufferLength, int1* memory, int4 memorySize);
	virtual ~CPUKeyboardController();
	virtual void ChangeKeyState(bool key_down, int4 key_code);
	void ActivityFunction();
};

#endif
