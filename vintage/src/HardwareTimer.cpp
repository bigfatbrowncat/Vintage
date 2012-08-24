#include "HardwareTimer.h"

void HardwareTimer::ActivityFunction()
{
	while (getState() == hdsOn)
	{
		if (isActive())
		{
			clock_t tt = clock();

			int1* stack = &getMemory()[getActivityContext().stackStart];
			int1* heap = &getMemory()[getActivityContext().heapStart];

			*(clock_t*)(&heap[0]) = tt;

			sendMessage(getActivityContext());
		}
    	usleep(100);	// 0.1 millisecond
	}
}
