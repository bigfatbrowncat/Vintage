#include "Console.h"

Console::Console(SDLScreen* window)
{
	this->window = window;
}

void Console::onMessageReceived(int4 port, int1* data, int4 length)
{
	int1 cutData[length + 1];
	switch (data[0])
	{
	case TERMINAL_CALL_PRINT:
		// getting the text to print and add null-termination
		memcpy(cutData, &data[1], length - 1);
		cutData[length - 1] = 0;
		cutData[length] = 0;

		// printing the text out
		window->Write((wchar_t*)cutData);
		break;

	case TERMINAL_CALL_MOVECURSOR:
		int2 x = *((int2*)&data[1]);
		int2 y = *((int2*)&data[3]);
		window->SetCursorPosition(x, y);
		break;
	}
}
