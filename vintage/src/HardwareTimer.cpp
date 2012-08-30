#include "HardwareTimer.h"

bool HardwareTimer::handleCommand(int4 command)
{
	// Handling base commands
	bool result = HardwareDevice::handleCommand(command);

	// Handling additional commands
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

	// Doing the activity
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

	return result;
}

