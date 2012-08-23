#include "HardwareTimer.h"

void HardwareTimer::ActivityFunction()
{
	while (getState() == hdsOn)
	{
		if (active)
		{
			clock_t tt = clock();

			int1* stack = &getMemory()[activityContext.stackStart];
			int1* heap = &getMemory()[activityContext.heapStart];
			*(clock_t*)(&heap[0]) = tt;
			broadcastMessage(activityContext);
		}
    	usleep(100);	// 0.1 millisecond
	}
}
