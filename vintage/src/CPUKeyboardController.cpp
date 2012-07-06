#include "CPUKeyboardController.h"

#include <pthread.h>

CPUKeyboardController::CPUKeyboardController(CPU& cpu, int port, int2 bufferLength):
	HardwareDevice(cpu, port),
	bufferLength(bufferLength)
{
	keyDown = new bool[bufferLength];
	//modifiers = new int2[bufferLength];
	keyCode = new int4[bufferLength];

	pthread_mutex_init(&keyBufferLock, NULL);

	current = 0; last = 0;
}

CPUKeyboardController::~CPUKeyboardController()
{
	pthread_mutex_destroy(&keyBufferLock);
	delete [] keyDown;
	//delete [] modifiers;
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
	while (!TurnOffPending())
	{
		// Taking the event away from the buffer.
		// This is synchronized with adding event to the buffer
		pthread_mutex_lock(&keyBufferLock);
		bool notEmpty = (current != last);
		int1 msg[5];
		if (notEmpty)
		{
			// First 4 bytes -- key code
			// 5th byte:
			//     0 bit - boolean - is key down

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
			GetCPU().handleInputPort(GetPort(), msg, 5);
		}
		else
		{
			usleep(100);	// 0.1 millisecond
		}
	}
}
