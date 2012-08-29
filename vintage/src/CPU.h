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
	pthread_mutex_t portReadingMutex;
	bool* inputPortIsWaiting;
	MessageContext* portInWaitingContext;
	list<MessageContext> contextStack;
	volatile bool someInputPortIsWaiting;
	pthread_t activity;
	Debugger* debugger;

protected:
	void ActivityFunction();

	void reportToDebugger(int1* stack, int4 stackPtr, int4 stackSize, int1* heap, int4 heapSize, int4 flow, FlowState state);
	void askDebugger(int1* stack, int4 stackPtr, int4 stackSize, int1* heap, int4 heapSize, int4 flow);

	virtual bool onMessageReceived(const MessageContext& context)
	{
		// TODO: No call to base method. This should be thinked over

		pthread_mutex_lock(&portReadingMutex);
		someInputPortIsWaiting = true;
		inputPortIsWaiting[context.port] = true;
		portInWaitingContext[context.port] = context;
		pthread_mutex_unlock(&portReadingMutex);
		return true;
	}
public:

	void setDebugger(Debugger& debugger)
	{
		this->debugger = &debugger;
	}

	CPU(int4 portsCount, int1* memory, int4 memorySize, const MessageContext& initialContext) :
		HardwareDevice(portsCount, memory, memorySize),
		debugger(NULL)
	{

		inputPortIsWaiting = new bool[portsCount];
		for (int i = 0; i < portsCount; i++) inputPortIsWaiting[i] = false;

		portInWaitingContext = new MessageContext[portsCount];

		someInputPortIsWaiting = false;
		contextStack.push_back(initialContext);

		pthread_mutex_init(&portReadingMutex, NULL);
	}

	virtual ~CPU()
	{
		//delete[] devices;
		delete[] inputPortIsWaiting;
		delete[] portInWaitingContext;

		pthread_mutex_destroy(&portReadingMutex);
	}

};

#endif /* CPU_H_ */
