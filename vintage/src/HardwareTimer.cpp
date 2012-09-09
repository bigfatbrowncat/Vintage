#include <unistd.h>
#include "HardwareTimer.h"

MessageHandlingResult HardwareTimer::handleMessage()
{
	// Handling base commands
	bool result = HardwareDevice::handleMessage();
	if (result == mhsUnsuccessful)
	{
		return mhsUnsuccessful;
	}

	int1* stack = &(getMemory()[contextStack.back().stackStart]);
	int1* heap = &(getMemory()[contextStack.back().heapStart]);

	// Handling additional commands
	int4 command = *((int4*)&stack[contextStack.back().stackPtr + 0]);
	if (command == HARDWARE_INITIALIZE)
	{
		// activityContext has already been
		// checked in HardwareDevice::handleMessage(),
		// so no double checking here
		if (activityContext.getFreeStackMemory() >= sizeof(clock_t))
		{
			activityContext.stackPtr -= sizeof(clock_t);
			return mhsSuccessful;
		}
		else
		{
			return mhsUnsuccessful;
		}
	}
	else
	{
		return mhsNotHandled;
	}
}

void HardwareTimer::doCycle(MessageHandlingResult handlingResult)
{
	if (activityContext.isValid(getMemorySize()) &&
	    activityContext.getAllocatedStackMemory() >= 2 * sizeof(int4))
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

		usleep(10000);	// sleeping for 10ms
	}

	if (contextStack.size() > 0)
	{
		// Do nothing with the input context. Just pass it.
		contextStack.pop_back();
	}
}
