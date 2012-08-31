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
	if (isActive())
	{
		clock_t tt = clock();

		int1* stack = &getMemory()[activityContext.stackStart];
		int1* heap = &getMemory()[activityContext.heapStart];

		// Getting address from the top of stack
		int4* target = ((int4*)&stack[activityContext.stackPtr]);

		// Writing the current clock data there
		*target = tt;

		sendMessage();
		result = true;
	}
	// Everything's done. Clearing the context
	contextStack.pop_back();
}
