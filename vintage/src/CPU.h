class CPU;

#ifndef CPU_H_
#define CPU_H_

#include <pthread.h>
#include <unistd.h>

#include "instructions.h"
#include "chain.h"
#include "HardwareDevice.h"
#include "Debugger.h"
#include "FlowState.h"

class CPU
{
	friend class HardwareDevice;
private:
	pthread_mutex_t portReadingMutex;

	int1* memory;
	int4 memorySize;

	int4 heapStart;
	int4 heapSize;

	int4 stackStart;
	int4 stackSize;
	int4 stackPtr;

	int4 portsCount;
	int4 portDataLength;

	HardwareDevice** devices;
	volatile int4* inputPortHandlersAddresses;

	bool* inputPortIsWaiting;
	int1* portInWaitingData;
	int4* portInWaitingDataLength;

	int4* inputPortsCurrentlyHandlingStack;
	volatile int4 inputPortsCurrentlyHandlingCount;

	volatile bool someInputPortIsWaiting;

	pthread_t activity;
	friend void* CPUActivityFunction(void* arg);

	Debugger* debugger;

	void ActivityFunction();
	volatile bool terminationPending;
	volatile bool terminated;

protected:
	void reportToDebugger(int1* stack, int4 stackPtr, int4 stackSize, int1* heap, int4 heapSize, int4 flow, FlowState state);
	void askDebugger(int1* stack, int4 stackPtr, int4 stackSize, int1* heap, int4 heapSize, int4 flow);

public:
	void handleInputPort(int4 port, const int1* data, int4 data_len)
	{
		pthread_mutex_lock(&portReadingMutex);
		someInputPortIsWaiting = true;
		inputPortIsWaiting[port] = true;
		memcpy(&portInWaitingData[port * portDataLength], data, data_len);
		portInWaitingDataLength[port] = data_len;
		pthread_mutex_unlock(&portReadingMutex);

		// Checking if we have a handler for the port
		if (inputPortHandlersAddresses[port] != 0)
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
			while (notHandledYet && !terminated);
		}
	}

	void TurnOff()
	{
		terminationPending = true;
	}

	bool isTerminated()
	{
		return terminated;
	}

	void setDebugger(Debugger& debugger)
	{
		this->debugger = &debugger;
	}

	CPU(int4 memorySize, int4 heapStart, int4 heapSize, int4 stackStart, int4 stackSize, int4 portsCount, int4 portDataLength) : debugger(NULL)
	{
		// Getting memory
		memory = new int1[memorySize];
		this->memorySize = memorySize;

		// Initiating heap
		this->heapStart = heapStart;
		this->heapSize = heapSize;

		// Initiating stack
		this->stackStart = stackStart;
		this->stackSize = stackSize;
		stackPtr = stackSize;

		this->portsCount = portsCount;
		this->portDataLength = portDataLength;

		devices = new HardwareDevice*[portsCount];
		inputPortHandlersAddresses = new int4[portsCount];
		for (int i = 0; i < portsCount; i++) inputPortHandlersAddresses[i] = 0;

		inputPortIsWaiting = new bool[portsCount];
		for (int i = 0; i < portsCount; i++) inputPortIsWaiting[i] = false;

		portInWaitingData = new int1[portsCount * portDataLength];
		portInWaitingDataLength = new int4[portsCount];

		someInputPortIsWaiting = false;

		inputPortsCurrentlyHandlingCount = 0;
		inputPortsCurrentlyHandlingStack = new int4[portsCount];


		pthread_mutex_init(&portReadingMutex, NULL);
	}

	int1* GetMemory()
	{
		return memory;
	}

	int4 GetMemorySize()
	{
		return memorySize;
	}

	virtual ~CPU()
	{
		delete[] memory;
		delete[] devices;
		delete[] inputPortHandlersAddresses;
		delete[] inputPortIsWaiting;
		delete[] portInWaitingData;
		delete[] portInWaitingDataLength;
		delete[] inputPortsCurrentlyHandlingStack;

		pthread_mutex_destroy(&portReadingMutex);
	}

	void TurnOn();
};

#endif /* CPU_H_ */
