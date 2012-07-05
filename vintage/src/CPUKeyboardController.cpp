#include "CPUKeyboardController.h"

#include <pthread.h>

CPUKeyboardController::CPUKeyboardController(CPU& cpu, int port, int2 bufferLength):
	HardwareDevice(cpu, port),
	bufferLength(bufferLength)
{
	keyDown = new int1[bufferLength];
	modifiers = new int2[bufferLength];
	keyCode = new int4[bufferLength];

	pthread_mutex_init(&keyBufferLock, NULL);

	current = 0; last = 0;
}

CPUKeyboardController::~CPUKeyboardController()
{
	pthread_mutex_destroy(&keyBufferLock);
	delete [] keyDown;
	delete [] modifiers;
	delete [] keyCode;
}

void CPUKeyboardController::ChangeKeyState(bool key_down, KeyModifiers modifiers, int4 key_code)
{
	pthread_mutex_lock(&keyBufferLock);
	if (last != current - 1)
	{
		this->keyDown[current] = key_down ? 1 : 0;
		this->modifiers[current] = modifiers;
		this->keyCode[current] = key_code;
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
		while (!TurnOffPending() && (current == last))
			usleep(100);	// 0.1 millisecond

		// Taking the event away from the buffer.
		// This is synchronized with adding event to the buffer
		pthread_mutex_lock(&keyBufferLock);
		bool notEmpty = (current != last);
		int1 msg[7];
		if (notEmpty)
		{
			msg[0] = keyDown[current];
			*((int2*)&msg[1]) = (int2)(modifiers[current]);
			*((int4*)&msg[3]) = keyCode[current];
			current = (current + 1) % bufferLength;
		}
		pthread_mutex_unlock(&keyBufferLock);

		// After taking the last event from the buffer,
		// we send it to the CPU.
		if (notEmpty)
		{
			GetCPU().handleInputPort(GetPort(), msg, 7);
		}

	}
}
