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
	void CallHandler(int1* heap, int1* stack, int4 stp);
public:
	Console(CPU& cpu, int port, SDLScreen* window);
};

#endif
