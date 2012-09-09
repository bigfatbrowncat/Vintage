#include "Console.h"

Console::Console(SDLScreen* window, int1* memory, int4 memorySize) :
	HardwareDevice(false, 1, memory, memorySize)
{
	this->window = window;
}

MessageHandlingResult Console::handleMessage()
{
	MessageHandlingResult result = HardwareDevice::handleMessage();
	if (result == mhsUnsuccessful)
	{
		return mhsUnsuccessful;
	}

	int1* stack = &(getMemory()[contextStack.back().stackStart]);
	int1* heap = &(getMemory()[contextStack.back().heapStart]);

	// Handling additional commands
	if (contextStack.back().getAllocatedStackMemory() < sizeof(int4))
	{
		return mhsUnsuccessful;	// Can't read the command
	}
	int4 command = *((int4*)&stack[contextStack.back().stackPtr + 0]);

	if (command == TERMINAL_PRINT)
	{
		if (contextStack.back().getAllocatedStackMemory() >= 2 * sizeof(int4))
		{
			int4 addr = *((int4*)&stack[contextStack.back().stackPtr + 4]);
			if (addr < contextStack.back().heapSize)
			{
				// TODO We have an issue here. We need to check the string length

				// printing the text out
				window->Write((wchar_t*)(&heap[addr]));

				return mhsSuccessful;
			}
			else
			{
				return mhsUnsuccessful;
			}
		}
		else
		{
			return mhsUnsuccessful;
		}
	}
	else if (command == TERMINAL_MOVECURSOR)
	{
		if (contextStack.back().getAllocatedStackMemory() >= sizeof(int4) + 2 * sizeof(int2))
		{
			int2 x = *((int2*)&stack[contextStack.back().stackPtr + 4]);
			int2 y = *((int2*)&stack[contextStack.back().stackPtr + 6]);
			window->SetCursorPosition(x, y);

			return mhsSuccessful;
		}
		else
		{
			return mhsUnsuccessful;
		}
	}

	return result;
}
