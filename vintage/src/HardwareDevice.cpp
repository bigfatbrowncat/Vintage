#include <time.h>
#include <unistd.h>

#include "HardwareDevice.h"


void* HardwareDevice_activity_function(void* arg)
{
	((HardwareDevice*)arg)->ActivityFunction();
	return NULL;
}


HardwareDevice::HardwareDevice(CPU& cpu, int port): cpu(cpu), port(port)
{
	isTurnedOff = true;
	cpu.devices[port] = this;
}

HardwareDevice::~HardwareDevice() {
	if (!isTurnedOff)
		TurnOff();
}

void HardwareDevice::TurnOn()
{
	if (isTurnedOff)
	{
		turnOffPending = false;
		isTurnedOff = false;
		pthread_create(&this->activity, NULL, &HardwareDevice_activity_function, this);
	}
}

void HardwareDevice::TurnOff()
{
	if (!isTurnedOff)
	{
		turnOffPending = true;
		void* value;
		pthread_join(activity, &value);
		isTurnedOff = true;
	}
}

void HardwareTimer::ActivityFunction()
{
	while (!TurnOffPending())
	{
    	clock_t tt = clock();
    	GetCPU().handleInputPort(GetPort(), (int1*)(&tt), 8);
    	usleep(100);	// 0.1 millisecond
	}
}

Console::Console(CPU& cpu, int port, SDLScreen* window): HardwareDevice(cpu, port)
{
	this->window = window;
}

void Console::CallHandler(int1* heap, int1* stack, int4 stp)
{
	int4 addr;
	switch (stack[stp])
	{
	case TERMINAL_CALL_PRINT:
		// here we get the memory address from the second parameter
		addr = *((int4*)&stack[stp + 4]);
		window->Write((wchar_t*)(&heap[addr]));
		break;
	case TERMINAL_CALL_MOVECURSOR:
		int2 x = *((int2*)&stack[stp + 4]);
		int2 y = *((int2*)&stack[stp + 6]);
		window->SetCursorPosition(x, y);
		break;
	}
}

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
		pthread_mutex_lock(&keyBufferLock);
		if (current != last)
		{
			int1 msg[7];
			msg[0] = keyDown[current];
			*((int2*)&msg[1]) = (int2)(modifiers[current]);
			*((int4*)&msg[3]) = keyCode[current];
			GetCPU().handleInputPort(GetPort(), msg, 7);
			current = (current + 1) % bufferLength;
		}
		pthread_mutex_unlock(&keyBufferLock);
	}
}


void DebuggerKeyboardController::ChangeKeyState(bool key_down, KeyModifiers modifiers, int4 key_code)
{
	if (key_down)
	{
		switch (key_code)
		{
		case SDLK_F1:
			debugger.run();
			break;
		case SDLK_F2:
			debugger.stop();
			break;
		case SDLK_F3:
			debugger.step();
			break;
		case SDLK_F4:
			debugger.halt();
			break;

		case SDLK_TAB:
			debugger.handleControlKey(ckTab);
			break;
		case SDLK_LEFT:
			debugger.handleControlKey(ckLeft);
			break;
		case SDLK_UP:
			debugger.handleControlKey(ckUp);
			break;
		case SDLK_RIGHT:
			debugger.handleControlKey(ckRight);
			break;
		case SDLK_DOWN:
			debugger.handleControlKey(ckDown);
			break;
		case SDLK_PAGEUP:
			debugger.handleControlKey(ckPageUp);
			break;
		case SDLK_PAGEDOWN:
			debugger.handleControlKey(ckPageDown);
			break;

		}

	}
}
