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
    	GetCPU().CallPortIn(GetPort(), (int1*)(&tt), 8);
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
			GetCPU().CallPortIn(GetPort(), msg, 7);
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

		case SDLK_INSERT:
			debugger.addWatch();
			break;
		case SDLK_0:
			if (debugger.isEditingWatchAddress())
				debugger.inputDigit(0);
			break;
		case SDLK_1:
			if (debugger.isEditingWatchAddress() || debugger.isEditingWatchLength())
				debugger.inputDigit(1);
			break;
		case SDLK_2:
			if (debugger.isEditingWatchAddress() || debugger.isEditingWatchLength())
				debugger.inputDigit(2);
			break;
		case SDLK_3:
			if (debugger.isEditingWatchAddress())
				debugger.inputDigit(3);
			break;
		case SDLK_4:
			if (debugger.isEditingWatchAddress() || debugger.isEditingWatchLength())
				debugger.inputDigit(4);
			break;
		case SDLK_5:
			if (debugger.isEditingWatchAddress())
				debugger.inputDigit(5);
			break;
		case SDLK_6:
			if (debugger.isEditingWatchAddress())
				debugger.inputDigit(6);
			break;
		case SDLK_7:
			if (debugger.isEditingWatchAddress())
				debugger.inputDigit(7);
			break;
		case SDLK_8:
			if (debugger.isEditingWatchAddress() || debugger.isEditingWatchLength())
				debugger.inputDigit(8);
			break;
		case SDLK_9:
			if (debugger.isEditingWatchAddress())
				debugger.inputDigit(9);
			break;
		case SDLK_a:
			if (debugger.isEditingWatchAddress())
				debugger.inputDigit(0xA);
			break;
		case SDLK_b:
			if (debugger.isEditingWatchAddress())
				debugger.inputDigit(0xB);
			break;
		case SDLK_c:
			if (debugger.isEditingWatchAddress())
				debugger.inputDigit(0xC);
			break;
		case SDLK_d:
			if (debugger.isEditingWatchAddress())
				debugger.inputDigit(0xD);
			break;
		case SDLK_e:
			if (debugger.isEditingWatchAddress())
				debugger.inputDigit(0xE);
			break;
		case SDLK_f:
			if (debugger.isEditingWatchAddress())
				debugger.inputDigit(0xF);
			break;
		case SDLK_SPACE:
			if (debugger.isEditingWatchAddress())
				debugger.switchRadix();
			break;
		case SDLK_TAB:
			debugger.changeSelection();
			break;
		case SDLK_BACKSPACE:
			if (debugger.isEditingWatchAddress())
				debugger.removeWatchAddressDigit();
			break;
		case SDLK_RETURN:
			debugger.completeAddWatch();
			break;
		}

	}
}
