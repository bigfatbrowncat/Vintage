class CPU;

#ifndef CPU_H_
#define CPU_H_

#include <pthread.h>

#include "instructions.h"
#include "chain.h"
#include "HardwareDevice.h"
#include "Debugger.h"


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
	int4* portInHandlers;

	bool* portInWaiting;
	int1* portInWaitingData;
	int4* portInWaitingDataLength;

	volatile int4 portInHandling;
	volatile bool anyPortInWaiting;
	volatile bool portInHandlingInProgress;

	pthread_t activity;
	friend void* CPU_activity_function(void* arg);

	Debugger* pDebugger;

	void ActivityFunction();
	volatile bool terminationPending;
	volatile bool halted;
public:
	void TurnOff()
	{
		terminationPending = true;
	}

	bool isHalted()
	{
		return halted;
	}

	void CallPortIn(int4 port, const int1* data, int4 data_len)
	{
		pthread_mutex_lock(&portReadingMutex);
		anyPortInWaiting = true;
		portInWaiting[port] = true;
		memcpy(&portInWaitingData[port * port_data_length], data, data_len);
		portInWaitingDataLength[port] = data_len;
		pthread_mutex_unlock(&portReadingMutex);
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
		portInHandlers = new int4[ports_count];
		for (int i = 0; i < ports_count; i++) portInHandlers[i] = 0;

		portInWaiting = new bool[ports_count];
		for (int i = 0; i < ports_count; i++) portInWaiting[i] = false;

		portInWaitingData = new int1[ports_count * port_data_length];
		portInWaitingDataLength = new int4[ports_count];

		anyPortInWaiting = false;
		portInHandlingInProgress = false;

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
		delete[] portInHandlers;
		delete[] portInWaiting;
		delete[] portInWaitingData;
		delete[] portInWaitingDataLength;

		pthread_mutex_destroy(&portReadingMutex);
	}

	void TurnOn();
};

#endif /* CPU_H_ */
