#include <unistd.h>
#include "HardwareTimer.h"

bool HardwareTimer::handleMessage()
{
	// Handling base commands
	bool result = HardwareDevice::handleMessage();

	int1* stack = &(getMemory()[contextStack.back().stackStart]);
	int1* heap = &(getMemory()[contextStack.back().heapStart]);

	// Handling additional commands
	int4 command = *((int4*)&stack[contextStack.back().stackPtr + 0]);
	if (command == HARDWARE_ACTIVATE)
	{
		activityContext.stackPtr -= sizeof(clock_t);
		result = true;	// Handled
	}
	else if (command == HARDWARE_DEACTIVATE)
	{
		activityContext.stackPtr += sizeof(clock_t);
		result = true;	// Handled
	}

	return result;
}

bool HardwareTimer::doCycle()
{
	bool result = false;
	if (contextStack.size() > 0)
	{
		// Do nothing with the input context. Just pass it.
		contextStack.pop_back();
		result = true;
	}

	if (isActive())
	{
		clock_t tt = clock();

		int1* stack = &getMemory()[activityContext.stackStart];
		int1* heap = &getMemory()[activityContext.heapStart];

		// Getting address from the top of stack
		int4* status = ((int4*)&stack[activityContext.stackPtr]);
		int4* value = ((int4*)&stack[activityContext.stackPtr + 4]);

		// Writing the current clock data there
		*status = HARDWARE_TIMER_REPORT_TIME;
		*value = tt;

		sendMessage();

		usleep(5000);	// sleeping for 5ms
		result = true;
	}
	return result;
}
