#include "HardwareTimer.h"

void HardwareTimer::ActivityFunction()
{
	while (getState() == hdsOn)
	{
    	clock_t tt = clock();
    	broadcastMessage((int1*)(&tt), 8);
    	usleep(100);	// 0.1 millisecond
	}
}
