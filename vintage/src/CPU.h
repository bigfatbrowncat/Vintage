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
	void reportToDebugger(int1* stack, int4 stackPtr, int4 stackSize, int1* heap, int4 heapSize, int4 flow, FlowState state);
	void askDebugger(int1* stack, int4 stackPtr, int4 stackSize, int1* heap, int4 heapSize, int4 flow);

	virtual bool handleMessage();
	virtual bool doCycle();

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
