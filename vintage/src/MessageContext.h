#ifndef MESSAGE_CONTEXT_H_
#define MESSAGE_CONTEXT_H_

struct MessageContext
{
	int4 port;

	int4 heapStart;
	int4 heapSize;

	int4 stackStart;
	int4 stackSize;
	int4 stackPtr;

	int4 flow;

	static int getSize()
	{
		return sizeof(port) + sizeof(heapStart) + sizeof(heapSize) +
		       sizeof(stackStart) + sizeof(stackSize) + sizeof(stackPtr) +
		       sizeof(flow);
	}

	MessageContext() :
		port(0), heapStart(0), heapSize(0), stackStart(0), stackSize(0), stackPtr(0), flow(0) { }


	MessageContext(int4 port, int4 heapStart, int4 heapSize, int4 stackStart, int4 stackSize, int4 stackPtr, int4 flow) :
		port(port), heapStart(heapStart), heapSize(heapSize), stackStart(stackStart), stackSize(stackSize), stackPtr(stackPtr), flow(flow) { }

	bool isValid(int memorySize)
	{
		int a1 = heapStart, a2 = heapStart + heapSize - 1;
		int b1 = stackStart, b2 = stackStart + stackSize - 1;
		if (a1 < 0 || b1 < 0 || a2 >= memorySize || b2 >= memorySize) return false;
		if (stackPtr < 0 || stackPtr > stackSize) return false;
		if (flow < 0 || flow >= heapSize) return false;
		return true;//(a2 < b1 || b2 < a1);
	}
	int getFreeStackMemory()
	{
		return stackPtr;
	}
	int getAllocatedStackMemory()
	{
		return stackSize - stackPtr;
	}

	void writeTo(int1* addr)
	{
		int4* p = (int4*)addr;
		p[0] = port;			// 0
		p[1] = heapStart;		// 4
		p[2] = heapSize;		// 8
		p[3] = stackStart;		// 12
		p[4] = stackSize;		// 16
		p[5] = stackPtr;		// 20
		p[6] = flow;			// 24
	}

	void readFrom(int1* addr)
	{
		int4* p = (int4*)addr;
		port = p[0];
		heapStart = p[1];
		heapSize = p[2];
		stackStart = p[3];
		stackSize = p[4];
		stackPtr = p[5];
		flow = p[6];
	}

};

#endif
