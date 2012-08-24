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

			// Getting address from the top of stack
			int addr = *((int4*)&stack[getActivityContext().stackPtr]);

			// Writing the current clock data there
			*(clock_t*)(&heap[addr]) = tt;

			sendMessage(getActivityContext());
		}
    	usleep(100);	// 0.1 millisecond
	}
}
