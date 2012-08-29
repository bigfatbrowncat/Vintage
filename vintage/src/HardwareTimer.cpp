#include "HardwareTimer.h"

bool HardwareTimer::doAction()
{
	clock_t tt = clock();

	int1* stack = &getMemory()[activityContext.stackStart];
	int1* heap = &getMemory()[activityContext.heapStart];

	// Getting address from the top of stack
	int4* target = ((int4*)&stack[activityContext.stackPtr]);

	// Writing the current clock data there
	*target = tt;

	sendMessage();
	return true;
}

bool HardwareTimer::onMessageReceived(const MessageContext& context)
{
	bool baseResult = HardwareDevice::onMessageReceived(context);

	int1* stack = &(getMemory()[context.stackStart]);
	int1* heap = &(getMemory()[context.heapStart]);

	int4 command = *((int4*)&stack[context.stackPtr + 0]);
	if (command == HARDWARE_ACTIVATE)
	{
		activityContext.stackPtr -= sizeof(clock_t);
		return true;	// Handled
	}
	else if (command == HARDWARE_DEACTIVATE)
	{
		activityContext.stackPtr += sizeof(clock_t);
		return true;	// Handled
	}
	else
	{
		return baseResult;	// Not handled. Maybe it's already handled in base...
	}
}
