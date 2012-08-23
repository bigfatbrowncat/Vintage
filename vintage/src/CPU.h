#ifndef CPU_H_
#define CPU_H_

#include <pthread.h>
#include <unistd.h>
#include <string>

#include "instructions.h"
#include "chain.h"
#include "HardwareDevice.h"
#include "Debugger.h"
#include "FlowState.h"
#include "CPUContext.h"

using namespace std;

struct PortInputHandler
{
	CPUContext context;
	bool assigned;
	PortInputHandler() : assigned(false) {}
};

class CPU : public HardwareDevice
{
private:
	pthread_mutex_t portReadingMutex;

	CPUContext initialContext;

	int4 portsCount;
	int4 portDataLength;

	//HardwareDevice** devices;
	PortInputHandler* portInputHandlers;

	bool* inputPortIsWaiting;
	int1* portInWaitingData;
	int4* portInWaitingDataLength;

	int4* inputPortsCurrentlyHandlingStack;
	volatile int4 inputPortsCurrentlyHandlingCount;

	volatile bool someInputPortIsWaiting;

	pthread_t activity;

	Debugger* debugger;

protected:
	void ActivityFunction();

	void reportToDebugger(int1* stack, int4 stackPtr, int4 stackSize, int1* heap, int4 heapSize, int4 flow, FlowState state);
	void askDebugger(int1* stack, int4 stackPtr, int4 stackSize, int1* heap, int4 heapSize, int4 flow);

	void onMessageReceived(int4 port, const int1* data, int4 data_len)
	{
		pthread_mutex_lock(&portReadingMutex);
		someInputPortIsWaiting = true;
		inputPortIsWaiting[port] = true;
		memcpy(&portInWaitingData[port * portDataLength], data, data_len);
		portInWaitingDataLength[port] = data_len;
		pthread_mutex_unlock(&portReadingMutex);

		// Checking if we have a handler for the port
		if (portInputHandlers[port].assigned)
		{
			// Waiting until for the CPU to handle the port
			bool notHandledYet;

			do
			{
				notHandledYet = false;
				pthread_mutex_lock(&portReadingMutex);
				// Checking if the port is still waiting to be handled
				if (inputPortIsWaiting[port]) notHandledYet = true;

				// Checking if our port handling is in progress
				for (int i = 0; i < inputPortsCurrentlyHandlingCount; i++)
				{
					if (inputPortsCurrentlyHandlingStack[i] == port)
					{
						notHandledYet = true;
					}
				}
				pthread_mutex_unlock(&portReadingMutex);

				usleep(100);
			}
			while (notHandledYet && getState() == hdsOn);
		}
	}
public:

	void setDebugger(Debugger& debugger)
	{
		this->debugger = &debugger;
	}

	CPU(int1* memory, int4 memorySize, const CPUContext& initialContext, int4 portsCount, int4 portDataLength) :
		HardwareDevice(memory, memorySize),
		initialContext(initialContext),
		debugger(NULL)
	{
		this->portsCount = portsCount;
		this->portDataLength = portDataLength;

		//devices = new HardwareDevice*[portsCount];
		portInputHandlers = new PortInputHandler[portsCount];

		inputPortIsWaiting = new bool[portsCount];
		for (int i = 0; i < portsCount; i++) inputPortIsWaiting[i] = false;

		portInWaitingData = new int1[portsCount * portDataLength];
		portInWaitingDataLength = new int4[portsCount];

		someInputPortIsWaiting = false;

		inputPortsCurrentlyHandlingCount = 0;
		inputPortsCurrentlyHandlingStack = new int4[portsCount];


		pthread_mutex_init(&portReadingMutex, NULL);
	}

	virtual ~CPU()
	{
		//delete[] devices;
		delete[] portInputHandlers;
		delete[] inputPortIsWaiting;
		delete[] portInWaitingData;
		delete[] portInWaitingDataLength;
		delete[] inputPortsCurrentlyHandlingStack;

		pthread_mutex_destroy(&portReadingMutex);
	}

};

#endif /* CPU_H_ */
