class CPU;

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

using namespace std;

struct CPUContext
{
	int4 heapStart;
	int4 heapSize;

	int4 stackStart;
	int4 stackSize;
	int4 stackPtr;

	int4 flow;

	int getSize()
	{
		return sizeof(heapStart) + sizeof(heapSize) +
		       sizeof(stackStart) + sizeof(stackSize) + sizeof(stackPtr) +
		       sizeof(flow);
	}

	CPUContext() {}

	CPUContext(int4 heapStart, int4 heapSize, int4 stackStart, int4 stackSize, int4 stackPtr, int4 flow) :
		heapStart(heapStart), heapSize(heapSize), stackStart(stackStart), stackSize(stackSize), stackPtr(stackPtr), flow(flow) { }

	void writeTo(int1* addr)
	{
		int4* p = (int4*)addr;
		p[0] = heapStart;
		p[1] = heapSize;
		p[2] = stackStart;
		p[3] = stackSize;
		p[4] = stackPtr;
		p[5] = flow;
	}

	void readFrom(int1* addr)
	{
		int4* p = (int4*)addr;
		heapStart = p[0];
		heapSize = p[1];
		stackStart = p[2];
		stackSize = p[3];
		stackPtr = p[4];
		flow = p[5];
	}

};

struct PortInputHandler
{
	CPUContext context;
	bool assigned;
	PortInputHandler() : assigned(false) {}
};

class CPU
{
	friend class HardwareDevice;
private:
	pthread_mutex_t portReadingMutex;

	int1* memory;
	int4 memorySize;

	CPUContext initialContext;

	int4 portsCount;
	int4 portDataLength;

	HardwareDevice** devices;
	PortInputHandler* portInputHandlers;

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

	CPU(int4 memorySize, int4 heapStart, int4 heapSize, int4 stackStart, int4 stackSize, int4 portsCount, int4 portDataLength) :
		debugger(NULL), initialContext(heapStart, heapSize, stackStart, stackSize, stackSize, 0)
	{
		// Getting memory
		memory = new int1[memorySize];
		this->memorySize = memorySize;

		this->portsCount = portsCount;
		this->portDataLength = portDataLength;

		devices = new HardwareDevice*[portsCount];
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
		delete[] portInputHandlers;
		delete[] inputPortIsWaiting;
		delete[] portInWaitingData;
		delete[] portInWaitingDataLength;
		delete[] inputPortsCurrentlyHandlingStack;

		pthread_mutex_destroy(&portReadingMutex);
	}

	void TurnOn();
};

#endif /* CPU_H_ */
