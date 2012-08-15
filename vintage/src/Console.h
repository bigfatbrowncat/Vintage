class Console;

#ifndef CONSOLE_H_
#define CONSOLE_H_

#include "SDLScreen.h"

class Console : public HardwareDevice
{
private:
	SDLScreen* window;
protected:
	void ActivityFunction() {}
	virtual void onMessageReceived(int4 port, int1* data, int4 length);

public:
	Console(SDLScreen* window);
	virtual ~Console() {}
};

#endif
