#include <stdio.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include <iomanip>

#include "instructions.h"
#include "CPU.h"
#include "Debugger.h"

#define GET_INSTR(instr, flow)		instr = *((instr_t*)(&heap[flow])); flow += sizeof(instr_t);
#define GET_ARG_INT4(arg, flow)		arg   = *((int4*)   (&heap[flow])); flow += sizeof(int4);

//#define OUTPUT_INSTRUCTIONS

void* CPUActivityFunction(void* arg)
{
	((CPU*)arg)->ActivityFunction();
	return NULL;
}

void CPU::TurnOn()
{
	terminationPending = false;
	pthread_create(&this->activity, NULL, &CPUActivityFunction, this);
}

void CPU::reportToDebugger(int1* stack, int4 stackPtr, int4 stackSize, int1* heap, int4 heapSize, int4 flow, FlowState state)
{
	if (debugger != NULL)
	{
		debugger->reportFlowStateChanged(state);
		debugger->reportCPUState(flow, &stack[stackPtr], stackSize, stackSize - stackPtr, heap, heapSize);
	}
}

void CPU::askDebugger(int1* stack, int4 stackPtr, int4 stackSize, int1* heap, int4 heapSize, int4 flow)
{
	if (debugger != NULL)
	{
		debugger->reportCPUState(flow, &stack[stackPtr], stackSize, stackSize - stackPtr, heap, heapSize);

		DebuggerOrder order;
		while (((order = debugger->askForOrder()) == doWait) && !terminationPending)
		{
			Sleep(10);
			debugger->reportCPUState(flow, &stack[stackPtr], stackSize, stackSize - stackPtr, heap, heapSize);
		}

		if (order == doHalt)
		{
			terminationPending = true;
		}
	}
}

void CPU::ActivityFunction()
{
	int4 flow = heapStart;
	int1* stack = &memory[stackStart];
	int1* heap = &memory[heapStart];

	bool portHandlingJustFinished = false;

	while (!terminationPending)
	{
		if (!portHandlingJustFinished)
		{
			// Checking ports input
			pthread_mutex_lock(&portReadingMutex);
			if (someInputPortIsWaiting)
			{
				// Clearing our flag (doing this by default, we will check it later)
				someInputPortIsWaiting = false;

				int inputPortsWaitingCount = 0;
				int portToHandle = -1;

				// Here we are checking if any port is waiting.
				// We don't consider ports, which numbers are greater or equal
				// than the currently handling one. This means that
				// the least is the index of the port, the most priority it has.

				// Example: if we are handling port #3 and port #4 is waiting,
				//          the handler for the port #4 will not be executed
				//          until port #3 handling is done.

				// And we start handling for the highest priority
				// (i.e. the least index) port
				for (int i = 0; i < portsCount; i++)
				{
					if (inputPortIsWaiting[i] && inputPortHandlersAddresses[i] > 0)
					{
						inputPortsWaitingCount++;
						if (portToHandle == -1 && i < inputPortsCurrentlyHandlingStack[inputPortsCurrentlyHandlingCount - 1])
						{
							// If it is the first port we found
							// and it's priority is greater than the currently handling ones,
							// saving it for handling NOW
							portToHandle = i;
						}
						else
						{
							// In all other cases it should be handled LATER,
							// so we have the flag to set
							someInputPortIsWaiting = true;

							// Nothing to search more
							break;
						}
					}
				}

				// Calling the handler
				if (portToHandle >= 0)		// it's better to use a flag here
				{
					// Adding the port we are handling to input-ports-we-are-currently-handling stack
					inputPortsCurrentlyHandlingStack[inputPortsCurrentlyHandlingCount] = portToHandle;
					inputPortsCurrentlyHandlingCount++;

					inputPortIsWaiting[portToHandle] = false;

					stackPtr -= 4;
					*((int4*)&stack[stackPtr]) = flow;

					flow = inputPortHandlersAddresses[portToHandle];

					stackPtr -= portInWaitingDataLength[portToHandle];

					for (int i = 0; i < portInWaitingDataLength[portToHandle]; i++)
					{
						*((int1*)&stack[stackPtr + i]) = *((int1*)&portInWaitingData[portToHandle * portDataLength + i]);
					}

					if (inputPortsWaitingCount == 1) someInputPortIsWaiting = false;

					// If we have just stepped into a handler, let's report the debugger about it
					reportToDebugger(stack, stackPtr, stackSize, heap, heapSize, flow, fsStepInHandler);
				}
			}
			pthread_mutex_unlock(&portReadingMutex);
		}
		else
		{
			portHandlingJustFinished = false;
		}

		askDebugger(stack, stackPtr, stackSize, heap, heapSize, flow);

#ifdef OUTPUT_INSTRUCTIONS
		printf("%d:\t", flow);
#endif
		int4 arg1, arg2, arg3;

		// Parsing
		instr_t instr;
		GET_INSTR(instr, flow);

		//instr_t instr = *((instr_t*)&heap[flow]);
		//flow += sizeof(instr_t);

		int4 tmpAddr;
		switch (instr)
		{
		case nop:
#ifdef OUTPUT_INSTRUCTIONS
			printf("nop");
			fflush(stdout);
#endif
			// Do nothing
			break;

		case alloc_const:
			GET_ARG_INT4(arg1, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("alloc %d", arg1);
			fflush(stdout);
#endif
			stackPtr -= arg1;
			break;

		case free_const:
			GET_ARG_INT4(arg1, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("free %d", arg1);
			fflush(stdout);
#endif
			stackPtr += arg1;
			break;

		case mov_stp_stp:
			GET_ARG_INT4(arg1, flow);
			GET_ARG_INT4(arg2, flow);
			GET_ARG_INT4(arg3, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("mov %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[stackPtr + arg2] = stack[stackPtr + arg3];
				break;
			case 2:
				*((int2*)&stack[stackPtr + arg2]) = *((int2*)&stack[stackPtr + arg3]);
				break;
			case 4:
				*((int4*)&stack[stackPtr + arg2]) = *((int4*)&stack[stackPtr + arg3]);
				break;
			case 8:
				*((int8*)&stack[stackPtr + arg2]) = *((int8*)&stack[stackPtr + arg3]);
				break;
			}

			break;

		case mov_stp_const:
			GET_ARG_INT4(arg1, flow);
			GET_ARG_INT4(arg2, flow);
			GET_ARG_INT4(arg3, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("mov %d, {%d}, %d", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[stackPtr + arg2] = (int1)arg3;
				break;
			case 2:
				*((int2*)&stack[stackPtr + arg2]) = (int2)arg3;
				break;
			case 4:
				*((int4*)&stack[stackPtr + arg2]) = (int4)arg3;
				break;
			case 8:
				*((int8*)&stack[stackPtr + arg2]) = (int8)arg3;
				break;
			}

			break;

		case mov_m_stp_stp:
			GET_ARG_INT4(arg1, flow);
			GET_ARG_INT4(arg2, flow);
			GET_ARG_INT4(arg3, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("mov %d, [{%d}], {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			tmpAddr = *((int4*)&stack[stackPtr + arg2]);

			switch (arg1)
			{
			case 1:
				heap[tmpAddr] = stack[stackPtr + arg3];
				break;
			case 2:
				*((int2*)&heap[tmpAddr]) = *((int2*)&stack[stackPtr + arg3]);
				break;
			case 4:
				*((int4*)&heap[tmpAddr]) = *((int4*)&stack[stackPtr + arg3]);
				break;
			case 8:
				*((int8*)&heap[tmpAddr]) = *((int8*)&stack[stackPtr + arg3]);
				break;
			}

			break;

		case mov_stp_m_stp:
			GET_ARG_INT4(arg1, flow);
			GET_ARG_INT4(arg2, flow);
			GET_ARG_INT4(arg3, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("mov %d, {%d}, [{%d}]", arg1, arg2, arg3);
			fflush(stdout);
#endif
			tmpAddr = *((int4*)&stack[stackPtr + arg3]);

			switch (arg1)
			{
			case 1:
				*((int1*)&stack[stackPtr + arg2]) = *((int1*)&heap[tmpAddr]);
				break;
			case 2:
				*((int2*)&stack[stackPtr + arg2]) = *((int2*)&heap[tmpAddr]);
				break;
			case 4:
				*((int4*)&stack[stackPtr + arg2]) = *((int4*)&heap[tmpAddr]);
				break;
			case 8:
				*((int8*)&stack[stackPtr + arg2]) = *((int8*)&heap[tmpAddr]);
				break;
			}

			break;


		case add_stp_stp:
			GET_ARG_INT4(arg1, flow);
			GET_ARG_INT4(arg2, flow);
			GET_ARG_INT4(arg3, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("add %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[stackPtr + arg2] += stack[stackPtr + arg3];
				break;
			case 2:
				*((int2*)&stack[stackPtr + arg2]) += *((int2*)&stack[stackPtr + arg3]);
				break;
			case 4:
				*((int4*)&stack[stackPtr + arg2]) += *((int4*)&stack[stackPtr + arg3]);
				break;
			case 8:
				*((int8*)&stack[stackPtr + arg2]) += *((int8*)&stack[stackPtr + arg3]);
				break;
			}

			break;

		case sub_stp_stp:
			GET_ARG_INT4(arg1, flow);
			GET_ARG_INT4(arg2, flow);
			GET_ARG_INT4(arg3, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("sub %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[stackPtr + arg2] -= stack[stackPtr + arg3];
				break;
			case 2:
				*((int2*)&stack[stackPtr + arg2]) -= *((int2*)&stack[stackPtr + arg3]);
				break;
			case 4:
				*((int4*)&stack[stackPtr + arg2]) -= *((int4*)&stack[stackPtr + arg3]);
				break;
			case 8:
				*((int8*)&stack[stackPtr + arg2]) -= *((int8*)&stack[stackPtr + arg3]);
				break;
			}

			break;

		case mul_stp_stp:
			GET_ARG_INT4(arg1, flow);
			GET_ARG_INT4(arg2, flow);
			GET_ARG_INT4(arg3, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("mul %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[stackPtr + arg2] *= stack[stackPtr + arg3];
				break;
			case 2:
				*((int2*)&stack[stackPtr + arg2]) *= *((int2*)&stack[stackPtr + arg3]);
				break;
			case 4:
				*((int4*)&stack[stackPtr + arg2]) *= *((int4*)&stack[stackPtr + arg3]);
				break;
			case 8:
				*((int8*)&stack[stackPtr + arg2]) *= *((int8*)&stack[stackPtr + arg3]);
				break;
			}

			break;

		case div_stp_stp:
			GET_ARG_INT4(arg1, flow);
			GET_ARG_INT4(arg2, flow);
			GET_ARG_INT4(arg3, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("div %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[stackPtr + arg2] /= stack[stackPtr + arg3];
				break;
			case 2:
				*((int2*)&stack[stackPtr + arg2]) /= *((int2*)&stack[stackPtr + arg3]);
				break;
			case 4:
				*((int4*)&stack[stackPtr + arg2]) /= *((int4*)&stack[stackPtr + arg3]);
				break;
			case 8:
				*((int8*)&stack[stackPtr + arg2]) /= *((int8*)&stack[stackPtr + arg3]);
				break;
			}

			break;

		case mod_stp_stp:
			GET_ARG_INT4(arg1, flow);
			GET_ARG_INT4(arg2, flow);
			GET_ARG_INT4(arg3, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("mod %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[stackPtr + arg2] %= stack[stackPtr + arg3];
				break;
			case 2:
				*((int2*)&stack[stackPtr + arg2]) %= *((int2*)&stack[stackPtr + arg3]);
				break;
			case 4:
				*((int4*)&stack[stackPtr + arg2]) %= *((int4*)&stack[stackPtr + arg3]);
				break;
			case 8:
				*((int8*)&stack[stackPtr + arg2]) %= *((int8*)&stack[stackPtr + arg3]);
				break;
			}

			break;

		case add_stp_const:
			GET_ARG_INT4(arg1, flow);
			GET_ARG_INT4(arg2, flow);
			GET_ARG_INT4(arg3, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("add %d, {%d}, %d", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[stackPtr + arg2] += (int1)arg3;
				break;
			case 2:
				*((int2*)&stack[stackPtr + arg2]) += (int2)arg3;
				break;
			case 4:
				*((int4*)&stack[stackPtr + arg2]) += (int4)arg3;
				break;
			case 8:
				*((int8*)&stack[stackPtr + arg2]) += (int8)arg3;
				break;
			}

			break;

		case sub_stp_const:
			GET_ARG_INT4(arg1, flow);
			GET_ARG_INT4(arg2, flow);
			GET_ARG_INT4(arg3, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("sub %d, {%d}, %d", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[stackPtr + arg2] -= (int1)arg3;
				break;
			case 2:
				*((int2*)&stack[stackPtr + arg2]) -= (int2)arg3;
				break;
			case 4:
				*((int4*)&stack[stackPtr + arg2]) -= (int4)arg3;
				break;
			case 8:
				*((int8*)&stack[stackPtr + arg2]) -= (int8)arg3;
				break;
			}

			break;

		case mul_stp_const:
			GET_ARG_INT4(arg1, flow);
			GET_ARG_INT4(arg2, flow);
			GET_ARG_INT4(arg3, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("mul %d, {%d}, %d", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[stackPtr + arg2] *= (int1)arg3;
				break;
			case 2:
				*((int2*)&stack[stackPtr + arg2]) *= (int2)arg3;
				break;
			case 4:
				*((int4*)&stack[stackPtr + arg2]) *= (int4)arg3;
				break;
			case 8:
				*((int8*)&stack[stackPtr + arg2]) *= (int8)arg3;
				break;
			}

			break;

		case div_stp_const:
			GET_ARG_INT4(arg1, flow);
			GET_ARG_INT4(arg2, flow);
			GET_ARG_INT4(arg3, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("div %d, {%d}, %d", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[stackPtr + arg2] /= (int1)arg3;
				break;
			case 2:
				*((int2*)&stack[stackPtr + arg2]) /= (int2)arg3;
				break;
			case 4:
				*((int4*)&stack[stackPtr + arg2]) /= (int4)arg3;
				break;
			case 8:
				*((int8*)&stack[stackPtr + arg2]) /= (int8)arg3;
				break;
			}

			break;

		case mod_stp_const:
			GET_ARG_INT4(arg1, flow);
			GET_ARG_INT4(arg2, flow);
			GET_ARG_INT4(arg3, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("mod %d, {%d}, %d", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[stackPtr + arg2] %= (int1)arg3;
				break;
			case 2:
				*((int2*)&stack[stackPtr + arg2]) %= (int2)arg3;
				break;
			case 4:
				*((int4*)&stack[stackPtr + arg2]) %= (int4)arg3;
				break;
			case 8:
				*((int8*)&stack[stackPtr + arg2]) %= (int8)arg3;
				break;
			}

			break;

		case not_stp:
			GET_ARG_INT4(arg1, flow);
			GET_ARG_INT4(arg2, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("not %d, {%d}", arg1, arg2);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[stackPtr + arg2] = !stack[stackPtr + arg2];
				break;
			case 2:
				*((int2*)&stack[stackPtr + arg2]) = ~(*((int2*)&stack[stackPtr + arg2]));
				break;
			case 4:
				*((int4*)&stack[stackPtr + arg2]) = ~(*((int4*)&stack[stackPtr + arg2]));
				break;
			case 8:
				*((int8*)&stack[stackPtr + arg2]) = ~(*((int8*)&stack[stackPtr + arg2]));
				break;
			}

			break;

		case and_stp_stp:
			GET_ARG_INT4(arg1, flow);
			GET_ARG_INT4(arg2, flow);
			GET_ARG_INT4(arg3, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("and %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[stackPtr + arg2] &= stack[stackPtr + arg3];
				break;
			case 2:
				*((int2*)&stack[stackPtr + arg2]) &= *((int2*)&stack[stackPtr + arg3]);
				break;
			case 4:
				*((int4*)&stack[stackPtr + arg2]) &= *((int4*)&stack[stackPtr + arg3]);
				break;
			case 8:
				*((int8*)&stack[stackPtr + arg2]) &= *((int8*)&stack[stackPtr + arg3]);
				break;
			}

			break;

		case or_stp_stp:
			GET_ARG_INT4(arg1, flow);
			GET_ARG_INT4(arg2, flow);
			GET_ARG_INT4(arg3, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("or %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[stackPtr + arg2] |= stack[stackPtr + arg3];
				break;
			case 2:
				*((int2*)&stack[stackPtr + arg2]) |= *((int2*)&stack[stackPtr + arg3]);
				break;
			case 4:
				*((int4*)&stack[stackPtr + arg2]) |= *((int4*)&stack[stackPtr + arg3]);
				break;
			case 8:
				*((int8*)&stack[stackPtr + arg2]) |= *((int8*)&stack[stackPtr + arg3]);
				break;
			}

			break;

		case xor_stp_stp:
			GET_ARG_INT4(arg1, flow);
			GET_ARG_INT4(arg2, flow);
			GET_ARG_INT4(arg3, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("xor %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[stackPtr + arg2] ^= stack[stackPtr + arg3];
				break;
			case 2:
				*((int2*)&stack[stackPtr + arg2]) ^= *((int2*)&stack[stackPtr + arg3]);
				break;
			case 4:
				*((int4*)&stack[stackPtr + arg2]) ^= *((int4*)&stack[stackPtr + arg3]);
				break;
			case 8:
				*((int8*)&stack[stackPtr + arg2]) ^= *((int8*)&stack[stackPtr + arg3]);
				break;
			}

			break;

		case and_stp_const:
			GET_ARG_INT4(arg1, flow);
			GET_ARG_INT4(arg2, flow);
			GET_ARG_INT4(arg3, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("and %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[stackPtr + arg2] &= arg3;
				break;
			case 2:
				*((int2*)&stack[stackPtr + arg2]) &= (int2)arg3;
				break;
			case 4:
				*((int4*)&stack[stackPtr + arg2]) &= (int4)arg3;
				break;
			case 8:
				*((int8*)&stack[stackPtr + arg2]) &= (int8)arg3;
				break;
			}

			break;

		case or_stp_const:
			GET_ARG_INT4(arg1, flow);
			GET_ARG_INT4(arg2, flow);
			GET_ARG_INT4(arg3, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("or %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[stackPtr + arg2] |= arg3;
				break;
			case 2:
				*((int2*)&stack[stackPtr + arg2]) |= (int2)arg3;
				break;
			case 4:
				*((int4*)&stack[stackPtr + arg2]) |= (int4)arg3;
				break;
			case 8:
				*((int8*)&stack[stackPtr + arg2]) |= (int8)arg3;
				break;
			}

			break;

		case xor_stp_const:
			GET_ARG_INT4(arg1, flow);
			GET_ARG_INT4(arg2, flow);
			GET_ARG_INT4(arg3, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("xor %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[stackPtr + arg2] ^= arg3;
				break;
			case 2:
				*((int2*)&stack[stackPtr + arg2]) ^= (int2)arg3;
				break;
			case 4:
				*((int4*)&stack[stackPtr + arg2]) ^= (int4)arg3;
				break;
			case 8:
				*((int8*)&stack[stackPtr + arg2]) ^= (int8)arg3;
				break;
			}

			break;

		case if_stp_flow:
			GET_ARG_INT4(arg1, flow);
			GET_ARG_INT4(arg2, flow);
			GET_ARG_INT4(arg3, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("if %d, {%d} [%d]", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				if (stack[stackPtr + arg2] != 0)
				{
					flow = arg3;
				}
				break;
			case 2:
				if (*((int2*)&stack[stackPtr + arg2]) != 0)
				{
					flow = arg3;
				}
				break;
			case 4:
				if (*((int4*)&stack[stackPtr + arg2]) != 0)
				{
					flow = arg3;
				}
				break;
			case 8:
				if (*((int8*)&stack[stackPtr + arg2]) != 0)
				{
					flow = arg3;
				}
				break;
			}
			break;

		case ifp_stp_flow:
			GET_ARG_INT4(arg1, flow);
			GET_ARG_INT4(arg2, flow);
			GET_ARG_INT4(arg3, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("ifp %d, {%d} [%d]", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				if (stack[stackPtr + arg2] > 0)
				{
					flow = arg3;
				}
				break;
			case 2:
				if (*((int2*)&stack[stackPtr + arg2]) > 0)
				{
					flow = arg3;
				}
				break;
			case 4:
				if (*((int4*)&stack[stackPtr + arg2]) > 0)
				{
					flow = arg3;
				}
				break;
			case 8:
				if (*((int8*)&stack[stackPtr + arg2]) > 0)
				{
					flow = arg3;
				}
				break;
			}
			break;


		case call_m_stp:
			GET_ARG_INT4(arg1, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("call [{%d}]", arg1);
			fflush(stdout);
#endif
			stackPtr -= 4;
			*((int4*)&stack[stackPtr]) = flow;
			flow = *((int4*)&stack[stackPtr + arg1]);

			reportToDebugger(stack, stackPtr, stackSize, heap, heapSize, flow, fsStepIn);
			break;

		case call_flow:
			GET_ARG_INT4(arg1, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("call [%d]", arg1);
			fflush(stdout);
#endif
			stackPtr -= 4;
			*((int4*)&stack[stackPtr]) = flow;
			flow = (int4)arg1;

			reportToDebugger(stack, stackPtr, stackSize, heap, heapSize, flow, fsStepIn);
			break;

		case ret_stp:
#ifdef OUTPUT_INSTRUCTIONS
			printf("ret");
			fflush(stdout);
#endif
			flow = *((int4*)&stack[stackPtr]);
			stackPtr += 4;	// removing the callr's address

			reportToDebugger(stack, stackPtr, stackSize, heap, heapSize, flow, fsStepOut);
			break;

		case hret_stp:
#ifdef OUTPUT_INSTRUCTIONS
			printf("hret");
			fflush(stdout);
#endif
			if (inputPortsCurrentlyHandlingCount > 0)
			{
				pthread_mutex_lock(&portReadingMutex);
				stackPtr += portInWaitingDataLength[inputPortsCurrentlyHandlingStack[inputPortsCurrentlyHandlingCount - 1]];	// removing port data
				flow = *((int4*)&stack[stackPtr]);
				stackPtr += 4;	// removing the callr's address
				inputPortsCurrentlyHandlingCount--;

				portHandlingJustFinished = true;
				reportToDebugger(stack, stackPtr, stackSize, heap, heapSize, flow, fsStepOutHandler);

				pthread_mutex_unlock(&portReadingMutex);
			}
			else
			{
				// TODO: Implement some exception here :)
			}

			break;

		case jmp_flow:
			GET_ARG_INT4(arg1, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("jmp [%d]", arg1);
			fflush(stdout);
#endif
			flow = arg1;
			break;

		case out_const:
			GET_ARG_INT4(arg1, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("out %d", arg1);
			fflush(stdout);
#endif
			devices[arg1]->CallHandler(heap, stack, stackPtr);
			break;

		case regin_const_flow:
			GET_ARG_INT4(arg1, flow);
			GET_ARG_INT4(arg2, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("regin %d, %d", arg1, arg2);
			fflush(stdout);
#endif
			inputPortHandlersAddresses[arg1] = arg2;
			break;

		case uregin_const:
			GET_ARG_INT4(arg1, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("uregin %d", arg1);
			fflush(stdout);
#endif
			inputPortHandlersAddresses[arg1] = 0;
			break;

		case halt:
#ifdef OUTPUT_INSTRUCTIONS
			printf("halt");
			fflush(stdout);
#endif
			// Halt
			terminationPending = true;		// Close the world...
			break;

		case ldcont_stp:
			GET_ARG_INT4(arg1, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("ldcont {%d}", arg1);
			fflush(stdout);
#endif

			break;

		case ldcont_m_stp:
			GET_ARG_INT4(arg1, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("ldcont [{%d}]", arg1);
			fflush(stdout);
#endif

			break;

		case svcont_stp:
			GET_ARG_INT4(arg1, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("svcont {%d}", arg1);
			fflush(stdout);
#endif

			break;

		case svcont_m_stp:
			GET_ARG_INT4(arg1, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("svcont [{%d}]", arg1);
			fflush(stdout);
#endif

			break;

		}

#ifdef OUTPUT_INSTRUCTIONS
		if (instr != jmp_flow)
		{
		printf("\t\t\t{");
		if (stackPtr < stackSize)  printf("%x", stack[stackPtr]);
		for (int i = stackPtr + 1; i < stackSize; i++)
			printf(", %x", stack[i]);
		printf("}\n");
		fflush(stdout);
		}
#endif
	}

	terminated = true;
}
