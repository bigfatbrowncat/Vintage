#include "Console.h"

Console::Console(SDLScreen* window, int1* memory, int4 memorySize) :
	HardwareDevice(1, memory, memorySize)
{
	this->window = window;
}

bool Console::onMessageReceived(const MessageContext& context)
{
	if (HardwareDevice::onMessageReceived(context))
	{
		return true; // Handled by the base class
	}
	else
	{
		int1* stack = &(getMemory()[context.stackStart]);
		int1* heap = &(getMemory()[context.heapStart]);

		int4 command = *((int4*)&stack[context.stackPtr + 0]);
		if (command == TERMINAL_PRINT)
		{
			int4 addr = *((int4*)&stack[context.stackPtr + 4]);

			// printing the text out
			window->Write((wchar_t*)(&heap[addr]));

			return true;	// Handled
		}
		else if (command == TERMINAL_MOVECURSOR)
		{
			int2 x = *((int2*)&stack[context.stackPtr + 4]);
			int2 y = *((int2*)&stack[context.stackPtr + 6]);
			window->SetCursorPosition(x, y);

			return true;	// Handled
		}
		else
		{
			return false;	// Not handled
		}
	}
}
