#include "Console.h"

Console::Console(SDLScreen* window, int1* memory, int4 memorySize) :
	HardwareDevice(memory, memorySize)
{
	this->window = window;
}

void Console::onMessageReceived(int4 port, const CPUContext& context)
{
	int1* stack = &(getMemory()[context.stackStart]);
	int1* heap = &(getMemory()[context.heapStart]);

	int1 command = stack[context.stackPtr + 0];
	if (command == TERMINAL_CALL_PRINT)
	{
		int4 addr = *((int4*)&stack[context.stackPtr + 4]);

		// printing the text out
		window->Write((wchar_t*)(&heap[addr]));
	}
	else if (command == TERMINAL_CALL_MOVECURSOR)
	{
		int2 x = *((int2*)&stack[context.stackPtr + 4]);
		int2 y = *((int2*)&stack[context.stackPtr + 6]);
		window->SetCursorPosition(x, y);
	}
}
