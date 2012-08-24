class Console;

#ifndef CONSOLE_H_
#define CONSOLE_H_

#include "SDLScreen.h"
#include "CPUContext.h"

#define	TERMINAL_PRINT								HARDWARE_CUSTOM + 0
#define	TERMINAL_MOVECURSOR							HARDWARE_CUSTOM + 1

class Console : public HardwareDevice
{
private:
	SDLScreen* window;
protected:
	void ActivityFunction() {}
	virtual bool onMessageReceived(const CPUContext& context);

public:
	Console(SDLScreen* window, int1* memory, int4 memorySize);
	virtual ~Console() {}
};

#endif
