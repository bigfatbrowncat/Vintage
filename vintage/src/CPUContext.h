#ifndef CPU_CONTEXT_H_
#define CPU_CONTEXT_H_

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

#endif
