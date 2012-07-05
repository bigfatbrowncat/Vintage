#include "HardwareTimer.h"

void HardwareTimer::ActivityFunction()
{
	while (!TurnOffPending())
	{
    	clock_t tt = clock();
    	GetCPU().handleInputPort(GetPort(), (int1*)(&tt), 8);
    	usleep(100);	// 0.1 millisecond
	}
}
