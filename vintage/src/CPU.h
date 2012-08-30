#ifndef CPU_H_
#define CPU_H_

#include <pthread.h>
#include <unistd.h>
#include <string>
#include <list>

#include "instructions.h"
#include "chain.h"
#include "HardwareDevice.h"
#include "Debugger.h"
#include "FlowState.h"
#include "MessageContext.h"

using namespace std;

class CPU : public HardwareDevice
{
private:
	Debugger* debugger;
	pthread_mutex_t portReadingMutex;

protected:
	virtual bool handleCommand(int4 command);

	void reportToDebugger(int1* stack, int4 stackPtr, int4 stackSize, int1* heap, int4 heapSize, int4 flow, FlowState state);
	void askDebugger(int1* stack, int4 stackPtr, int4 stackSize, int1* heap, int4 heapSize, int4 flow);

	/*
	virtual bool onMessageReceived(const MessageContext& context)
	{
		pthread_mutex_lock(&portReadingMutex);
		// Adding the context of the port we are handling to the contexts stack
		contextStack.push_back(context);
		// Selecting the new context
		int1* stack = &(getMemory()[contextStack.back().stackStart]);
		int1* heap = &(getMemory()[contextStack.back().heapStart]);

		// As far as we have just stepped into a handler, let's report the debugger about it
		reportToDebugger(stack, contextStack.back().stackPtr, contextStack.back().stackSize, heap, contextStack.back().heapSize, contextStack.back().flow, fsStepInHandler);

		pthread_mutex_unlock(&portReadingMutex);

		return true;
	}*/

public:

	void setDebugger(Debugger& debugger)
	{
		this->debugger = &debugger;
	}

	CPU(int4 portsCount, int1* memory, int4 memorySize, const MessageContext& initialContext) :
		HardwareDevice(true, portsCount, memory, memorySize),
		debugger(NULL)
	{
		pthread_mutex_init(&portReadingMutex, NULL);
		contextStack.push_back(initialContext);
	}

	virtual ~CPU()
	{
		pthread_mutex_destroy(&portReadingMutex);
	}

};

#endif /* CPU_H_ */
