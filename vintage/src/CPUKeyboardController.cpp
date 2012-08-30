#include "CPUKeyboardController.h"

#include <pthread.h>
#include <unistd.h>

CPUKeyboardController::CPUKeyboardController(int4 bufferLength, int1* memory, int4 memorySize) :
	HardwareDevice(false, 1, memory, memorySize),
	bufferLength(bufferLength),
	current(0), last(0)
{
	keyDown = new bool[bufferLength];
	keyCode = new int4[bufferLength];

	pthread_mutex_init(&keyBufferLock, NULL);
}

CPUKeyboardController::~CPUKeyboardController()
{
	pthread_mutex_destroy(&keyBufferLock);
	delete [] keyDown;
	delete [] keyCode;
}

void CPUKeyboardController::processKeyEvent(bool key_down, int4 key_code)
{
	pthread_mutex_lock(&keyBufferLock);
	if (last != current - 1)
	{
		this->keyDown[last] = key_down;
		this->keyCode[last] = key_code;
		last = (last + 1) % bufferLength;
	}
	else
	{
		// TODO: buffer overflow here
	}
	pthread_mutex_unlock(&keyBufferLock);
}

bool CPUKeyboardController::handleCommand(int4 command)
{
	bool result = HardwareDevice::handleCommand(command);

	if (command == HARDWARE_ACTIVATE)
	{
		activityContext.stackPtr -= 5; // keyboard message size
		result = true;	// Handled
	}
	else if (command == HARDWARE_DEACTIVATE)
	{
		activityContext.stackPtr += 5; // keyboard message size
		result = true;	// Handled
	}

	if (isActive())
	{
		// Taking the event away from the buffer.
		// This is synchronized with adding event to the buffer
		pthread_mutex_lock(&keyBufferLock);
		bool notEmpty = (current != last);
		if (notEmpty)
		{
			// First 4 bytes -- key code
			// 5th byte:
			//     0 bit - boolean - is key down
			//   1-7 bit - unused

			int1* stack = &getMemory()[activityContext.stackStart];
			int1* heap = &getMemory()[activityContext.heapStart];

			int1* msg = &stack[activityContext.stackPtr];
			*((int4*)&msg[0]) = keyCode[current];
			*((int1*)&msg[4]) = keyDown[current] ? 1 : 0;
			printf("<- (%d, %d) %d %d\n", current, last, keyCode[current], keyDown[current]);
			current = (current + 1) % bufferLength;
		}
		pthread_mutex_unlock(&keyBufferLock);

		// After taking the last event from the buffer,
		// we send it to the CPU.
		if (notEmpty)
		{
			sendMessage();
			result = true;
		}
	}

	contextStack.pop_back();
	return result;
}

