#include "Console.h"

Console::Console(SDLScreen* window, int1* memory, int4 memorySize) :
	HardwareDevice(false, 1, memory, memorySize)
{
	this->window = window;
}

bool Console::handleCommand(int4 command)
{
	bool result = HardwareDevice::handleCommand(command);

	int1* stack = &(getMemory()[contextStack.back().stackStart]);
	int1* heap = &(getMemory()[contextStack.back().heapStart]);

	if (command == TERMINAL_PRINT)
	{
		int4 addr = *((int4*)&stack[contextStack.back().stackPtr + 4]);

		// printing the text out
		window->Write((wchar_t*)(&heap[addr]));

		result = true;	// Handled
	}
	else if (command == TERMINAL_MOVECURSOR)
	{
		int2 x = *((int2*)&stack[contextStack.back().stackPtr + 4]);
		int2 y = *((int2*)&stack[contextStack.back().stackPtr + 6]);
		window->SetCursorPosition(x, y);

		result = true;	// Handled
	}

	return result;
}
