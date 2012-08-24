#include "CPUKeyboardController.h"

#include <pthread.h>
#include <unistd.h>

CPUKeyboardController::CPUKeyboardController(int4 bufferLength, int1* memory, int4 memorySize) :
	bufferLength(bufferLength),
	HardwareDevice(memory, memorySize),
	active(false)
{
	keyDown = new bool[bufferLength];
	keyCode = new int4[bufferLength];

	pthread_mutex_init(&keyBufferLock, NULL);

	current = 0; last = 0;
}

CPUKeyboardController::~CPUKeyboardController()
{
	pthread_mutex_destroy(&keyBufferLock);
	delete [] keyDown;
	delete [] keyCode;
}

void CPUKeyboardController::ChangeKeyState(bool key_down, int4 key_code)
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

void CPUKeyboardController::ActivityFunction()
{
	while (getState() == hdsOn)
	{
		// Taking the event away from the buffer.
		// This is synchronized with adding event to the buffer
		pthread_mutex_lock(&keyBufferLock);
		bool notEmpty = (current != last);
		if (notEmpty && active)
		{
			// First 4 bytes -- key code
			// 5th byte:
			//     0 bit - boolean - is key down
			//   1-7 bit - unused

			int1* heap = &(getMemory()[activityContext.heapStart]);
			int1* msg = &heap[0];
			*((int4*)&msg[0]) = keyCode[current];
			*((int1*)&msg[4]) = keyDown[current] ? 1 : 0;
			printf("<- (%d, %d) %d %d\n", current, last, keyCode[current], keyDown[current]);
			current = (current + 1) % bufferLength;
		}
		pthread_mutex_unlock(&keyBufferLock);

		// After taking the last event from the buffer,
		// we send it to the CPU.
		if (notEmpty && active)
		{
			sendMessage(getActivityContext());
		}
		else
		{
			usleep(100);	// 0.1 millisecond
		}
	}
}
