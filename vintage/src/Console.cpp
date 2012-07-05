#include "Console.h"

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
