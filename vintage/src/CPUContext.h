#ifndef CPU_CONTEXT_H_
#define CPU_CONTEXT_H_

struct CPUContext
{
	int4 port;

	int4 heapStart;
	int4 heapSize;

	int4 stackStart;
	int4 stackSize;
	int4 stackPtr;

	int4 flow;

	int getSize()
	{
		return sizeof(port) + sizeof(heapStart) + sizeof(heapSize) +
		       sizeof(stackStart) + sizeof(stackSize) + sizeof(stackPtr) +
		       sizeof(flow);
	}

	CPUContext() {}

	CPUContext(int4 port, int4 heapStart, int4 heapSize, int4 stackStart, int4 stackSize, int4 stackPtr, int4 flow) :
		port(port), heapStart(heapStart), heapSize(heapSize), stackStart(stackStart), stackSize(stackSize), stackPtr(stackPtr), flow(flow) { }

	void writeTo(int1* addr)
	{
		int4* p = (int4*)addr;
		p[0] = port;
		p[1] = heapStart;
		p[2] = heapSize;
		p[3] = stackStart;
		p[4] = stackSize;
		p[5] = stackPtr;
		p[6] = flow;
	}

	void readFrom(int1* addr)
	{
		int4* p = (int4*)addr;
		port = p[0];
		heapStart = p[1];
		heapSize = p[2];
		stackStart = p[2];
		stackSize = p[3];
		stackPtr = p[4];
		flow = p[5];
	}

};

#endif
