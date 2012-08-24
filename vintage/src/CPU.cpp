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

#define OUTPUT_INSTRUCTIONS

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
		while (((order = debugger->askForOrder()) == doWait) && getState() == hdsOn)
		{
			Sleep(10);
			debugger->reportCPUState(flow, &stack[stackPtr], stackSize, stackSize - stackPtr, heap, heapSize);
		}

		if (order == doHalt && getState() == hdsOn)
		{
			issueTurningOff();
		}
	}
}

void CPU::ActivityFunction()
{
	CPUContext context = initialContext;
	CPUContext contextToSend;
	int1* stack = &(getMemory()[context.stackStart]);
	int1* heap = &(getMemory()[context.heapStart]);

	bool portHandlingJustFinished = false;

	while (getState() == hdsOn)
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
					if (inputPortIsWaiting[i])
					{
						inputPortsWaitingCount++;
						if (portToHandle == -1 && i < contextStack.back().port)
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
					// Adding the context of the port we are handling to the contexts stack
					contextStack.push_back(portInWaitingContext[portToHandle]);

					inputPortIsWaiting[portToHandle] = false;

					// Backing up the current context
					CPUContext tmpContext = context;

					// Selecting the new context
					context = portInWaitingContext[portToHandle];
					stack = &(getMemory()[context.stackStart]);
					heap = &(getMemory()[context.heapStart]);

					// If it was the last port to handle, clearing the flag
					if (inputPortsWaitingCount == 1) someInputPortIsWaiting = false;

					// As far as we have just stepped into a handler, let's report the debugger about it
					reportToDebugger(stack, context.stackPtr, context.stackSize, heap, context.heapSize, context.flow, fsStepInHandler);
				}
			}
			pthread_mutex_unlock(&portReadingMutex);
		}
		else
		{
			portHandlingJustFinished = false;
		}

		askDebugger(stack, context.stackPtr, context.stackSize, heap, context.heapSize, context.flow);

#ifdef OUTPUT_INSTRUCTIONS
		printf("%d:\t", context.flow);
#endif
		int4 arg1, arg2, arg3;

		// Parsing
		instr_t instr;
		GET_INSTR(instr, context.flow);

		//instr_t instr = *((instr_t*)&heap[context.flow]);
		//context.flow += sizeof(instr_t);

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
			GET_ARG_INT4(arg1, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("alloc %d", arg1);
			fflush(stdout);
#endif
			context.stackPtr -= arg1;
			break;

		case free_const:
			GET_ARG_INT4(arg1, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("free %d", arg1);
			fflush(stdout);
#endif
			context.stackPtr += arg1;
			break;

		case mov_stp_stp:
			GET_ARG_INT4(arg1, context.flow);
			GET_ARG_INT4(arg2, context.flow);
			GET_ARG_INT4(arg3, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("mov %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[context.stackPtr + arg2] = stack[context.stackPtr + arg3];
				break;
			case 2:
				*((int2*)&stack[context.stackPtr + arg2]) = *((int2*)&stack[context.stackPtr + arg3]);
				break;
			case 4:
				*((int4*)&stack[context.stackPtr + arg2]) = *((int4*)&stack[context.stackPtr + arg3]);
				break;
			case 8:
				*((int8*)&stack[context.stackPtr + arg2]) = *((int8*)&stack[context.stackPtr + arg3]);
				break;
			}

			break;

		case mov_stp_const:
			GET_ARG_INT4(arg1, context.flow);
			GET_ARG_INT4(arg2, context.flow);
			GET_ARG_INT4(arg3, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("mov %d, {%d}, %d", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[context.stackPtr + arg2] = (int1)arg3;
				break;
			case 2:
				*((int2*)&stack[context.stackPtr + arg2]) = (int2)arg3;
				break;
			case 4:
				*((int4*)&stack[context.stackPtr + arg2]) = (int4)arg3;
				break;
			case 8:
				*((int8*)&stack[context.stackPtr + arg2]) = (int8)arg3;
				break;
			}

			break;

		case mov_m_stp_stp:
			GET_ARG_INT4(arg1, context.flow);
			GET_ARG_INT4(arg2, context.flow);
			GET_ARG_INT4(arg3, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("mov %d, [{%d}], {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			tmpAddr = *((int4*)&stack[context.stackPtr + arg2]);

			switch (arg1)
			{
			case 1:
				heap[tmpAddr] = stack[context.stackPtr + arg3];
				break;
			case 2:
				*((int2*)&heap[tmpAddr]) = *((int2*)&stack[context.stackPtr + arg3]);
				break;
			case 4:
				*((int4*)&heap[tmpAddr]) = *((int4*)&stack[context.stackPtr + arg3]);
				break;
			case 8:
				*((int8*)&heap[tmpAddr]) = *((int8*)&stack[context.stackPtr + arg3]);
				break;
			}

			break;

		case mov_stp_m_stp:
			GET_ARG_INT4(arg1, context.flow);
			GET_ARG_INT4(arg2, context.flow);
			GET_ARG_INT4(arg3, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("mov %d, {%d}, [{%d}]", arg1, arg2, arg3);
			fflush(stdout);
#endif
			tmpAddr = *((int4*)&stack[context.stackPtr + arg3]);

			switch (arg1)
			{
			case 1:
				*((int1*)&stack[context.stackPtr + arg2]) = *((int1*)&heap[tmpAddr]);
				break;
			case 2:
				*((int2*)&stack[context.stackPtr + arg2]) = *((int2*)&heap[tmpAddr]);
				break;
			case 4:
				*((int4*)&stack[context.stackPtr + arg2]) = *((int4*)&heap[tmpAddr]);
				break;
			case 8:
				*((int8*)&stack[context.stackPtr + arg2]) = *((int8*)&heap[tmpAddr]);
				break;
			}

			break;


		case add_stp_stp:
			GET_ARG_INT4(arg1, context.flow);
			GET_ARG_INT4(arg2, context.flow);
			GET_ARG_INT4(arg3, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("add %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[context.stackPtr + arg2] += stack[context.stackPtr + arg3];
				break;
			case 2:
				*((int2*)&stack[context.stackPtr + arg2]) += *((int2*)&stack[context.stackPtr + arg3]);
				break;
			case 4:
				*((int4*)&stack[context.stackPtr + arg2]) += *((int4*)&stack[context.stackPtr + arg3]);
				break;
			case 8:
				*((int8*)&stack[context.stackPtr + arg2]) += *((int8*)&stack[context.stackPtr + arg3]);
				break;
			}

			break;

		case sub_stp_stp:
			GET_ARG_INT4(arg1, context.flow);
			GET_ARG_INT4(arg2, context.flow);
			GET_ARG_INT4(arg3, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("sub %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[context.stackPtr + arg2] -= stack[context.stackPtr + arg3];
				break;
			case 2:
				*((int2*)&stack[context.stackPtr + arg2]) -= *((int2*)&stack[context.stackPtr + arg3]);
				break;
			case 4:
				*((int4*)&stack[context.stackPtr + arg2]) -= *((int4*)&stack[context.stackPtr + arg3]);
				break;
			case 8:
				*((int8*)&stack[context.stackPtr + arg2]) -= *((int8*)&stack[context.stackPtr + arg3]);
				break;
			}

			break;

		case mul_stp_stp:
			GET_ARG_INT4(arg1, context.flow);
			GET_ARG_INT4(arg2, context.flow);
			GET_ARG_INT4(arg3, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("mul %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[context.stackPtr + arg2] *= stack[context.stackPtr + arg3];
				break;
			case 2:
				*((int2*)&stack[context.stackPtr + arg2]) *= *((int2*)&stack[context.stackPtr + arg3]);
				break;
			case 4:
				*((int4*)&stack[context.stackPtr + arg2]) *= *((int4*)&stack[context.stackPtr + arg3]);
				break;
			case 8:
				*((int8*)&stack[context.stackPtr + arg2]) *= *((int8*)&stack[context.stackPtr + arg3]);
				break;
			}

			break;

		case div_stp_stp:
			GET_ARG_INT4(arg1, context.flow);
			GET_ARG_INT4(arg2, context.flow);
			GET_ARG_INT4(arg3, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("div %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[context.stackPtr + arg2] /= stack[context.stackPtr + arg3];
				break;
			case 2:
				*((int2*)&stack[context.stackPtr + arg2]) /= *((int2*)&stack[context.stackPtr + arg3]);
				break;
			case 4:
				*((int4*)&stack[context.stackPtr + arg2]) /= *((int4*)&stack[context.stackPtr + arg3]);
				break;
			case 8:
				*((int8*)&stack[context.stackPtr + arg2]) /= *((int8*)&stack[context.stackPtr + arg3]);
				break;
			}

			break;

		case mod_stp_stp:
			GET_ARG_INT4(arg1, context.flow);
			GET_ARG_INT4(arg2, context.flow);
			GET_ARG_INT4(arg3, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("mod %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[context.stackPtr + arg2] %= stack[context.stackPtr + arg3];
				break;
			case 2:
				*((int2*)&stack[context.stackPtr + arg2]) %= *((int2*)&stack[context.stackPtr + arg3]);
				break;
			case 4:
				*((int4*)&stack[context.stackPtr + arg2]) %= *((int4*)&stack[context.stackPtr + arg3]);
				break;
			case 8:
				*((int8*)&stack[context.stackPtr + arg2]) %= *((int8*)&stack[context.stackPtr + arg3]);
				break;
			}

			break;

		case add_stp_const:
			GET_ARG_INT4(arg1, context.flow);
			GET_ARG_INT4(arg2, context.flow);
			GET_ARG_INT4(arg3, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("add %d, {%d}, %d", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[context.stackPtr + arg2] += (int1)arg3;
				break;
			case 2:
				*((int2*)&stack[context.stackPtr + arg2]) += (int2)arg3;
				break;
			case 4:
				*((int4*)&stack[context.stackPtr + arg2]) += (int4)arg3;
				break;
			case 8:
				*((int8*)&stack[context.stackPtr + arg2]) += (int8)arg3;
				break;
			}

			break;

		case sub_stp_const:
			GET_ARG_INT4(arg1, context.flow);
			GET_ARG_INT4(arg2, context.flow);
			GET_ARG_INT4(arg3, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("sub %d, {%d}, %d", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[context.stackPtr + arg2] -= (int1)arg3;
				break;
			case 2:
				*((int2*)&stack[context.stackPtr + arg2]) -= (int2)arg3;
				break;
			case 4:
				*((int4*)&stack[context.stackPtr + arg2]) -= (int4)arg3;
				break;
			case 8:
				*((int8*)&stack[context.stackPtr + arg2]) -= (int8)arg3;
				break;
			}

			break;

		case mul_stp_const:
			GET_ARG_INT4(arg1, context.flow);
			GET_ARG_INT4(arg2, context.flow);
			GET_ARG_INT4(arg3, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("mul %d, {%d}, %d", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[context.stackPtr + arg2] *= (int1)arg3;
				break;
			case 2:
				*((int2*)&stack[context.stackPtr + arg2]) *= (int2)arg3;
				break;
			case 4:
				*((int4*)&stack[context.stackPtr + arg2]) *= (int4)arg3;
				break;
			case 8:
				*((int8*)&stack[context.stackPtr + arg2]) *= (int8)arg3;
				break;
			}

			break;

		case div_stp_const:
			GET_ARG_INT4(arg1, context.flow);
			GET_ARG_INT4(arg2, context.flow);
			GET_ARG_INT4(arg3, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("div %d, {%d}, %d", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[context.stackPtr + arg2] /= (int1)arg3;
				break;
			case 2:
				*((int2*)&stack[context.stackPtr + arg2]) /= (int2)arg3;
				break;
			case 4:
				*((int4*)&stack[context.stackPtr + arg2]) /= (int4)arg3;
				break;
			case 8:
				*((int8*)&stack[context.stackPtr + arg2]) /= (int8)arg3;
				break;
			}

			break;

		case mod_stp_const:
			GET_ARG_INT4(arg1, context.flow);
			GET_ARG_INT4(arg2, context.flow);
			GET_ARG_INT4(arg3, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("mod %d, {%d}, %d", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[context.stackPtr + arg2] %= (int1)arg3;
				break;
			case 2:
				*((int2*)&stack[context.stackPtr + arg2]) %= (int2)arg3;
				break;
			case 4:
				*((int4*)&stack[context.stackPtr + arg2]) %= (int4)arg3;
				break;
			case 8:
				*((int8*)&stack[context.stackPtr + arg2]) %= (int8)arg3;
				break;
			}

			break;

		case not_stp:
			GET_ARG_INT4(arg1, context.flow);
			GET_ARG_INT4(arg2, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("not %d, {%d}", arg1, arg2);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[context.stackPtr + arg2] = ~stack[context.stackPtr + arg2];
				break;
			case 2:
				*((int2*)&stack[context.stackPtr + arg2]) = ~(*((int2*)&stack[context.stackPtr + arg2]));
				break;
			case 4:
				*((int4*)&stack[context.stackPtr + arg2]) = ~(*((int4*)&stack[context.stackPtr + arg2]));
				break;
			case 8:
				*((int8*)&stack[context.stackPtr + arg2]) = ~(*((int8*)&stack[context.stackPtr + arg2]));
				break;
			}

			break;

		case and_stp_stp:
			GET_ARG_INT4(arg1, context.flow);
			GET_ARG_INT4(arg2, context.flow);
			GET_ARG_INT4(arg3, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("and %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[context.stackPtr + arg2] &= stack[context.stackPtr + arg3];
				break;
			case 2:
				*((int2*)&stack[context.stackPtr + arg2]) &= *((int2*)&stack[context.stackPtr + arg3]);
				break;
			case 4:
				*((int4*)&stack[context.stackPtr + arg2]) &= *((int4*)&stack[context.stackPtr + arg3]);
				break;
			case 8:
				*((int8*)&stack[context.stackPtr + arg2]) &= *((int8*)&stack[context.stackPtr + arg3]);
				break;
			}

			break;

		case or_stp_stp:
			GET_ARG_INT4(arg1, context.flow);
			GET_ARG_INT4(arg2, context.flow);
			GET_ARG_INT4(arg3, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("or %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[context.stackPtr + arg2] |= stack[context.stackPtr + arg3];
				break;
			case 2:
				*((int2*)&stack[context.stackPtr + arg2]) |= *((int2*)&stack[context.stackPtr + arg3]);
				break;
			case 4:
				*((int4*)&stack[context.stackPtr + arg2]) |= *((int4*)&stack[context.stackPtr + arg3]);
				break;
			case 8:
				*((int8*)&stack[context.stackPtr + arg2]) |= *((int8*)&stack[context.stackPtr + arg3]);
				break;
			}

			break;

		case xor_stp_stp:
			GET_ARG_INT4(arg1, context.flow);
			GET_ARG_INT4(arg2, context.flow);
			GET_ARG_INT4(arg3, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("xor %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[context.stackPtr + arg2] ^= stack[context.stackPtr + arg3];
				break;
			case 2:
				*((int2*)&stack[context.stackPtr + arg2]) ^= *((int2*)&stack[context.stackPtr + arg3]);
				break;
			case 4:
				*((int4*)&stack[context.stackPtr + arg2]) ^= *((int4*)&stack[context.stackPtr + arg3]);
				break;
			case 8:
				*((int8*)&stack[context.stackPtr + arg2]) ^= *((int8*)&stack[context.stackPtr + arg3]);
				break;
			}

			break;

		case and_stp_const:
			GET_ARG_INT4(arg1, context.flow);
			GET_ARG_INT4(arg2, context.flow);
			GET_ARG_INT4(arg3, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("and %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[context.stackPtr + arg2] &= arg3;
				break;
			case 2:
				*((int2*)&stack[context.stackPtr + arg2]) &= (int2)arg3;
				break;
			case 4:
				*((int4*)&stack[context.stackPtr + arg2]) &= (int4)arg3;
				break;
			case 8:
				*((int8*)&stack[context.stackPtr + arg2]) &= (int8)arg3;
				break;
			}

			break;

		case or_stp_const:
			GET_ARG_INT4(arg1, context.flow);
			GET_ARG_INT4(arg2, context.flow);
			GET_ARG_INT4(arg3, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("or %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[context.stackPtr + arg2] |= arg3;
				break;
			case 2:
				*((int2*)&stack[context.stackPtr + arg2]) |= (int2)arg3;
				break;
			case 4:
				*((int4*)&stack[context.stackPtr + arg2]) |= (int4)arg3;
				break;
			case 8:
				*((int8*)&stack[context.stackPtr + arg2]) |= (int8)arg3;
				break;
			}

			break;

		case xor_stp_const:
			GET_ARG_INT4(arg1, context.flow);
			GET_ARG_INT4(arg2, context.flow);
			GET_ARG_INT4(arg3, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("xor %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[context.stackPtr + arg2] ^= arg3;
				break;
			case 2:
				*((int2*)&stack[context.stackPtr + arg2]) ^= (int2)arg3;
				break;
			case 4:
				*((int4*)&stack[context.stackPtr + arg2]) ^= (int4)arg3;
				break;
			case 8:
				*((int8*)&stack[context.stackPtr + arg2]) ^= (int8)arg3;
				break;
			}

			break;

		case if_stp_flow:
			GET_ARG_INT4(arg1, context.flow);
			GET_ARG_INT4(arg2, context.flow);
			GET_ARG_INT4(arg3, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("if %d, {%d} [%d]", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				if (stack[context.stackPtr + arg2] != 0)
				{
					context.flow = arg3;
				}
				break;
			case 2:
				if (*((int2*)&stack[context.stackPtr + arg2]) != 0)
				{
					context.flow = arg3;
				}
				break;
			case 4:
				if (*((int4*)&stack[context.stackPtr + arg2]) != 0)
				{
					context.flow = arg3;
				}
				break;
			case 8:
				if (*((int8*)&stack[context.stackPtr + arg2]) != 0)
				{
					context.flow = arg3;
				}
				break;
			}
			break;

		case ifp_stp_flow:
			GET_ARG_INT4(arg1, context.flow);
			GET_ARG_INT4(arg2, context.flow);
			GET_ARG_INT4(arg3, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("ifp %d, {%d} [%d]", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				if (stack[context.stackPtr + arg2] > 0)
				{
					context.flow = arg3;
				}
				break;
			case 2:
				if (*((int2*)&stack[context.stackPtr + arg2]) > 0)
				{
					context.flow = arg3;
				}
				break;
			case 4:
				if (*((int4*)&stack[context.stackPtr + arg2]) > 0)
				{
					context.flow = arg3;
				}
				break;
			case 8:
				if (*((int8*)&stack[context.stackPtr + arg2]) > 0)
				{
					context.flow = arg3;
				}
				break;
			}
			break;


		case call_stp:
			GET_ARG_INT4(arg1, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("call [{%d}]", arg1);
			fflush(stdout);
#endif
			context.stackPtr -= 4;
			*((int4*)&stack[context.stackPtr]) = context.flow;
			context.flow = *((int4*)&stack[context.stackPtr + arg1]);

			reportToDebugger(stack, context.stackPtr, context.stackSize, heap, context.heapSize, context.flow, fsStepIn);
			break;

		case call_flow:
			GET_ARG_INT4(arg1, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("call [%d]", arg1);
			fflush(stdout);
#endif
			context.stackPtr -= 4;
			*((int4*)&stack[context.stackPtr]) = context.flow;
			context.flow = (int4)arg1;

			reportToDebugger(stack, context.stackPtr, context.stackSize, heap, context.heapSize, context.flow, fsStepIn);
			break;

		case ret_stp:
#ifdef OUTPUT_INSTRUCTIONS
			printf("ret");
			fflush(stdout);
#endif
			context.flow = *((int4*)&stack[context.stackPtr]);
			context.stackPtr += 4;	// removing the caller's address

			reportToDebugger(stack, context.stackPtr, context.stackSize, heap, context.heapSize, context.flow, fsStepOut);
			break;

		case hret_stp:
#ifdef OUTPUT_INSTRUCTIONS
			printf("hret");
			fflush(stdout);
#endif
			if (contextStack.size() > 1)
			{
				pthread_mutex_lock(&portReadingMutex);

				// Removing the last context from the context stack
				contextStack.pop_back();

				// Setting the next context from the stack as the current
				context = contextStack.back();
				stack = &(getMemory()[context.stackStart]);
				heap = &(getMemory()[context.heapStart]);

				portHandlingJustFinished = true;
				reportToDebugger(stack, context.stackPtr, context.stackSize, heap, context.heapSize, context.flow, fsStepOutHandler);

				pthread_mutex_unlock(&portReadingMutex);
			}
			else
			{
				// TODO: Put some exception code here :)
			}

			break;

		case jmp_flow:
			GET_ARG_INT4(arg1, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("jmp [%d]", arg1);
			fflush(stdout);
#endif
			context.flow = arg1;
			break;

		case out_const:
			GET_ARG_INT4(arg1, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("out %d", arg1);
			fflush(stdout);
#endif
			contextToSend = context;
			contextToSend.port = arg1;
			sendMessage(contextToSend);
			break;

		case out_stp:
			GET_ARG_INT4(arg1, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("out {%d}", arg1);
			fflush(stdout);
#endif
			contextToSend = context;
			contextToSend.port = *((int4*)&stack[context.stackPtr + arg1]);
			sendMessage(contextToSend);
			break;

		case halt:
#ifdef OUTPUT_INSTRUCTIONS
			printf("halt");
			fflush(stdout);
#endif
			// Halt
			issueTurningOff();		// Close the world...
			break;

		case setcont_stp:
			GET_ARG_INT4(arg1, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("setcont {%d}", arg1);
			fflush(stdout);
#endif
			context.readFrom(&stack[context.stackPtr + arg1]);
			stack = &(getMemory()[context.stackStart]);
			heap = &(getMemory()[context.heapStart]);

			break;

		case setcont_m_stp:
			GET_ARG_INT4(arg1, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("setcont [{%d}]", arg1);
			fflush(stdout);
#endif

			tmpAddr = *((int4*)&stack[context.stackPtr + arg1]);
			context.readFrom(((int1*)&heap[tmpAddr]));
			stack = &(getMemory()[context.stackStart]);
			heap = &(getMemory()[context.heapStart]);

			break;

		case getcont_stp:
			GET_ARG_INT4(arg1, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("getcont {%d}", arg1);
			fflush(stdout);
#endif

			context.writeTo(&stack[context.stackPtr + arg1]);

			break;

		case getcont_m_stp:
			GET_ARG_INT4(arg1, context.flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("getcont [{%d}]", arg1);
			fflush(stdout);
#endif

			tmpAddr = *((int4*)&stack[context.stackPtr + arg1]);
			context.writeTo(((int1*)&heap[tmpAddr]));

			break;

		}

#ifdef OUTPUT_INSTRUCTIONS
		printf("\t\t\t{");
		if (context.stackPtr < context.stackSize)  printf("%d", stack[context.stackPtr]);
		for (unsigned int i = context.stackPtr + 1; i < context.stackSize; i++)
			printf(", %d", stack[i]);
		printf("}\n");
		fflush(stdout);
#endif
	}
}
