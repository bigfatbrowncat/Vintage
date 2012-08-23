class Console;

#ifndef CONSOLE_H_
#define CONSOLE_H_

#include "SDLScreen.h"
#include "CPUContext.h"

class Console : public HardwareDevice
{
private:
	SDLScreen* window;
protected:
	void ActivityFunction() {}
	virtual void onMessageReceived(int4 port, const CPUContext& context);

public:
	Console(SDLScreen* window, int1* memory, int4 memorySize);
	virtual ~Console() {}
};

#endif
