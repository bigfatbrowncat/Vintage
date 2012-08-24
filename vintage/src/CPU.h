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
#include "CPUContext.h"

using namespace std;

class CPU : public HardwareDevice
{
private:
	pthread_mutex_t portReadingMutex;

	CPUContext initialContext;

	int4 portsCount;

	bool* inputPortIsWaiting;
	CPUContext* portInWaitingContext;

	list<CPUContext> contextStack;

	volatile bool someInputPortIsWaiting;

	pthread_t activity;

	Debugger* debugger;

protected:
	void ActivityFunction();

	void reportToDebugger(int1* stack, int4 stackPtr, int4 stackSize, int1* heap, int4 heapSize, int4 flow, FlowState state);
	void askDebugger(int1* stack, int4 stackPtr, int4 stackSize, int1* heap, int4 heapSize, int4 flow);

	virtual void onMessageReceived(int4 port, const CPUContext& context)
	{
		pthread_mutex_lock(&portReadingMutex);
		someInputPortIsWaiting = true;
		inputPortIsWaiting[port] = true;
		portInWaitingContext[port] = context;
		pthread_mutex_unlock(&portReadingMutex);
	}
public:

	void setDebugger(Debugger& debugger)
	{
		this->debugger = &debugger;
	}

	CPU(int1* memory, int4 memorySize, const CPUContext& initialContext, int4 portsCount) :
		HardwareDevice(memory, memorySize),
		initialContext(initialContext),
		debugger(NULL)
	{
		this->portsCount = portsCount;

		inputPortIsWaiting = new bool[portsCount];
		for (int i = 0; i < portsCount; i++) inputPortIsWaiting[i] = false;

		portInWaitingContext = new CPUContext[portsCount];

		someInputPortIsWaiting = false;

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
