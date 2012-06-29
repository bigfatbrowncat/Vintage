class CPU;

#ifndef CPU_H_
#define CPU_H_

#include <pthread.h>
#include <unistd.h>

#include "instructions.h"
#include "chain.h"
#include "HardwareDevice.h"
#include "Debugger.h"

#define NO_INDEX		-1

class CPU
{
	friend class HardwareDevice;
private:
	pthread_mutex_t portReadingMutex;

	int1* heap;
	int4 heapSize;

	int1* stack;
	int4 stackSize;

	int4 stackPtr;

	int4 ports_count;
	int4 port_data_length;

	HardwareDevice** devices;
	volatile int4* inputPortHandlersAddresses;

	bool* inputPortIsWaiting;
	int1* portInWaitingData;
	int4* portInWaitingDataLength;

	int4* inputPortsCurrentlyHandlingStack;
	volatile int4 inputPortsCurrentlyHandlingCount;

	volatile bool someInputPortIsWaiting;

	pthread_t activity;
	friend void* CPU_activity_function(void* arg);

	Debugger* pDebugger;

	void ActivityFunction();
	volatile bool terminationPending;
	volatile bool terminated;
public:
	void TurnOff()
	{
		terminationPending = true;
	}

	bool isTerminated()
	{
		return terminated;
	}

	void handleInputPort(int4 port, const int1* data, int4 data_len)
	{
		pthread_mutex_lock(&portReadingMutex);
		someInputPortIsWaiting = true;
		inputPortIsWaiting[port] = true;
		memcpy(&portInWaitingData[port * port_data_length], data, data_len);
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

	void setDebugger(Debugger& debugger) { this->pDebugger = &debugger; }

	CPU(int4 heap_size, int4 stack_size, int4 ports_count, int4 port_data_length) : pDebugger(NULL)
	{
		heap = new int1[heap_size];
		this->heapSize = heap_size;
		heap[0] = halt;

		stack = new int1[stack_size];
		this->stackSize = stack_size;

		this->ports_count = ports_count;
		this->port_data_length = port_data_length;

		devices = new HardwareDevice*[ports_count];
		inputPortHandlersAddresses = new int4[ports_count];
		for (int i = 0; i < ports_count; i++) inputPortHandlersAddresses[i] = 0;

		inputPortIsWaiting = new bool[ports_count];
		for (int i = 0; i < ports_count; i++) inputPortIsWaiting[i] = false;

		portInWaitingData = new int1[ports_count * port_data_length];
		portInWaitingDataLength = new int4[ports_count];

		someInputPortIsWaiting = false;

		inputPortsCurrentlyHandlingCount = 0;
		inputPortsCurrentlyHandlingStack = new int4[ports_count];

		stackPtr = stack_size;

		pthread_mutex_init(&portReadingMutex, NULL);
	}

	int1* GetHeap()
	{
		return heap;
	}

	int4 GetHeapSize()
	{
		return heapSize;
	}

	virtual ~CPU()
	{
		delete[] heap;
		delete[] stack;
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
