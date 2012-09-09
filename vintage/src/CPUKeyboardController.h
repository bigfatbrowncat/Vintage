#ifndef CPUKEYBOARDCONTROLLER_H_
#define CPUKEYBOARDCONTROLLER_H_

#include "KeyModifiers.h"
#include "HardwareDevice.h"
#include "KeyboardController.h"
#include "MessageContext.h"

#include <pthread.h>

#define HARDWARE_KEYBOARD_KEY_EVENT				HARDWARE_CUSTOM + 0

class CPUKeyboardController : public HardwareDevice, public KeyboardController
{
private:
	pthread_mutex_t keyBufferLock;

	int4 bufferLength;

	volatile int4 current, last;
	bool* keyDown;
	int4* keyCode;

protected:
	virtual MessageHandlingResult handleMessage();
	virtual void doCycle(MessageHandlingResult handlingResult);

public:
	CPUKeyboardController(int4 bufferLength, int1* memory, int4 memorySize);
	virtual ~CPUKeyboardController();

	virtual void processKeyEvent(bool key_down, int4 key_code);
};

#endif
