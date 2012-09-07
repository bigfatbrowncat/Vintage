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

bool CPU::handleMessage()
{
	// Selecting the new context
	int1* stack = &(getMemory()[contextStack.back().stackStart]);
	int1* heap = &(getMemory()[contextStack.back().heapStart]);

	// As far as we have just stepped into a handler, let's report the debugger about it
	reportToDebugger(stack, contextStack.back().stackPtr, contextStack.back().stackSize, heap, contextStack.back().heapSize, contextStack.back().flow, fsStepInHandler);
	return true;
}

bool CPU::doCycle()
{
	int1* stack = &(getMemory()[contextStack.back().stackStart]);
	int1* heap = &(getMemory()[contextStack.back().heapStart]);



		askDebugger(stack, contextStack.back().stackPtr, contextStack.back().stackSize, heap, contextStack.back().heapSize, contextStack.back().flow);

#ifdef OUTPUT_INSTRUCTIONS
		printf("%d:\t", contextStack.back().flow);
#endif
		int4 arg1, arg2, arg3;

		// Parsing
		instr_t instr;
		GET_INSTR(instr, contextStack.back().flow);

		//instr_t instr = *((instr_t*)&heap[contextStack.back().flow]);
		//contextStack.back().flow += sizeof(instr_t);

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
			GET_ARG_INT4(arg1, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("alloc %d", arg1);
			fflush(stdout);
#endif
			contextStack.back().stackPtr -= arg1;
			break;

		case free_const:
			GET_ARG_INT4(arg1, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("free %d", arg1);
			fflush(stdout);
#endif
			contextStack.back().stackPtr += arg1;
			break;

		case mov_stp_stp:
			GET_ARG_INT4(arg1, contextStack.back().flow);
			GET_ARG_INT4(arg2, contextStack.back().flow);
			GET_ARG_INT4(arg3, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("mov %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[contextStack.back().stackPtr + arg2] = stack[contextStack.back().stackPtr + arg3];
				break;
			case 2:
				*((int2*)&stack[contextStack.back().stackPtr + arg2]) = *((int2*)&stack[contextStack.back().stackPtr + arg3]);
				break;
			case 4:
				*((int4*)&stack[contextStack.back().stackPtr + arg2]) = *((int4*)&stack[contextStack.back().stackPtr + arg3]);
				break;
			case 8:
				*((int8*)&stack[contextStack.back().stackPtr + arg2]) = *((int8*)&stack[contextStack.back().stackPtr + arg3]);
				break;
			}

			break;

		case mov_stp_const:
			GET_ARG_INT4(arg1, contextStack.back().flow);
			GET_ARG_INT4(arg2, contextStack.back().flow);
			GET_ARG_INT4(arg3, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("mov %d, {%d}, %d", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[contextStack.back().stackPtr + arg2] = (int1)arg3;
				break;
			case 2:
				*((int2*)&stack[contextStack.back().stackPtr + arg2]) = (int2)arg3;
				break;
			case 4:
				*((int4*)&stack[contextStack.back().stackPtr + arg2]) = (int4)arg3;
				break;
			case 8:
				*((int8*)&stack[contextStack.back().stackPtr + arg2]) = (int8)arg3;
				break;
			}

			break;

		case mov_m_stp_stp:
			GET_ARG_INT4(arg1, contextStack.back().flow);
			GET_ARG_INT4(arg2, contextStack.back().flow);
			GET_ARG_INT4(arg3, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("mov %d, [{%d}], {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			tmpAddr = *((int4*)&stack[contextStack.back().stackPtr + arg2]);

			switch (arg1)
			{
			case 1:
				heap[tmpAddr] = stack[contextStack.back().stackPtr + arg3];
				break;
			case 2:
				*((int2*)&heap[tmpAddr]) = *((int2*)&stack[contextStack.back().stackPtr + arg3]);
				break;
			case 4:
				*((int4*)&heap[tmpAddr]) = *((int4*)&stack[contextStack.back().stackPtr + arg3]);
				break;
			case 8:
				*((int8*)&heap[tmpAddr]) = *((int8*)&stack[contextStack.back().stackPtr + arg3]);
				break;
			}

			break;

		case mov_stp_m_stp:
			GET_ARG_INT4(arg1, contextStack.back().flow);
			GET_ARG_INT4(arg2, contextStack.back().flow);
			GET_ARG_INT4(arg3, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("mov %d, {%d}, [{%d}]", arg1, arg2, arg3);
			fflush(stdout);
#endif
			tmpAddr = *((int4*)&stack[contextStack.back().stackPtr + arg3]);

			switch (arg1)
			{
			case 1:
				*((int1*)&stack[contextStack.back().stackPtr + arg2]) = *((int1*)&heap[tmpAddr]);
				break;
			case 2:
				*((int2*)&stack[contextStack.back().stackPtr + arg2]) = *((int2*)&heap[tmpAddr]);
				break;
			case 4:
				*((int4*)&stack[contextStack.back().stackPtr + arg2]) = *((int4*)&heap[tmpAddr]);
				break;
			case 8:
				*((int8*)&stack[contextStack.back().stackPtr + arg2]) = *((int8*)&heap[tmpAddr]);
				break;
			}

			break;


		case add_stp_stp:
			GET_ARG_INT4(arg1, contextStack.back().flow);
			GET_ARG_INT4(arg2, contextStack.back().flow);
			GET_ARG_INT4(arg3, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("add %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[contextStack.back().stackPtr + arg2] += stack[contextStack.back().stackPtr + arg3];
				break;
			case 2:
				*((int2*)&stack[contextStack.back().stackPtr + arg2]) += *((int2*)&stack[contextStack.back().stackPtr + arg3]);
				break;
			case 4:
				*((int4*)&stack[contextStack.back().stackPtr + arg2]) += *((int4*)&stack[contextStack.back().stackPtr + arg3]);
				break;
			case 8:
				*((int8*)&stack[contextStack.back().stackPtr + arg2]) += *((int8*)&stack[contextStack.back().stackPtr + arg3]);
				break;
			}

			break;

		case sub_stp_stp:
			GET_ARG_INT4(arg1, contextStack.back().flow);
			GET_ARG_INT4(arg2, contextStack.back().flow);
			GET_ARG_INT4(arg3, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("sub %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[contextStack.back().stackPtr + arg2] -= stack[contextStack.back().stackPtr + arg3];
				break;
			case 2:
				*((int2*)&stack[contextStack.back().stackPtr + arg2]) -= *((int2*)&stack[contextStack.back().stackPtr + arg3]);
				break;
			case 4:
				*((int4*)&stack[contextStack.back().stackPtr + arg2]) -= *((int4*)&stack[contextStack.back().stackPtr + arg3]);
				break;
			case 8:
				*((int8*)&stack[contextStack.back().stackPtr + arg2]) -= *((int8*)&stack[contextStack.back().stackPtr + arg3]);
				break;
			}

			break;

		case mul_stp_stp:
			GET_ARG_INT4(arg1, contextStack.back().flow);
			GET_ARG_INT4(arg2, contextStack.back().flow);
			GET_ARG_INT4(arg3, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("mul %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[contextStack.back().stackPtr + arg2] *= stack[contextStack.back().stackPtr + arg3];
				break;
			case 2:
				*((int2*)&stack[contextStack.back().stackPtr + arg2]) *= *((int2*)&stack[contextStack.back().stackPtr + arg3]);
				break;
			case 4:
				*((int4*)&stack[contextStack.back().stackPtr + arg2]) *= *((int4*)&stack[contextStack.back().stackPtr + arg3]);
				break;
			case 8:
				*((int8*)&stack[contextStack.back().stackPtr + arg2]) *= *((int8*)&stack[contextStack.back().stackPtr + arg3]);
				break;
			}

			break;

		case div_stp_stp:
			GET_ARG_INT4(arg1, contextStack.back().flow);
			GET_ARG_INT4(arg2, contextStack.back().flow);
			GET_ARG_INT4(arg3, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("div %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[contextStack.back().stackPtr + arg2] /= stack[contextStack.back().stackPtr + arg3];
				break;
			case 2:
				*((int2*)&stack[contextStack.back().stackPtr + arg2]) /= *((int2*)&stack[contextStack.back().stackPtr + arg3]);
				break;
			case 4:
				*((int4*)&stack[contextStack.back().stackPtr + arg2]) /= *((int4*)&stack[contextStack.back().stackPtr + arg3]);
				break;
			case 8:
				*((int8*)&stack[contextStack.back().stackPtr + arg2]) /= *((int8*)&stack[contextStack.back().stackPtr + arg3]);
				break;
			}

			break;

		case mod_stp_stp:
			GET_ARG_INT4(arg1, contextStack.back().flow);
			GET_ARG_INT4(arg2, contextStack.back().flow);
			GET_ARG_INT4(arg3, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("mod %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[contextStack.back().stackPtr + arg2] %= stack[contextStack.back().stackPtr + arg3];
				break;
			case 2:
				*((int2*)&stack[contextStack.back().stackPtr + arg2]) %= *((int2*)&stack[contextStack.back().stackPtr + arg3]);
				break;
			case 4:
				*((int4*)&stack[contextStack.back().stackPtr + arg2]) %= *((int4*)&stack[contextStack.back().stackPtr + arg3]);
				break;
			case 8:
				*((int8*)&stack[contextStack.back().stackPtr + arg2]) %= *((int8*)&stack[contextStack.back().stackPtr + arg3]);
				break;
			}

			break;

		case add_stp_const:
			GET_ARG_INT4(arg1, contextStack.back().flow);
			GET_ARG_INT4(arg2, contextStack.back().flow);
			GET_ARG_INT4(arg3, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("add %d, {%d}, %d", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[contextStack.back().stackPtr + arg2] += (int1)arg3;
				break;
			case 2:
				*((int2*)&stack[contextStack.back().stackPtr + arg2]) += (int2)arg3;
				break;
			case 4:
				*((int4*)&stack[contextStack.back().stackPtr + arg2]) += (int4)arg3;
				break;
			case 8:
				*((int8*)&stack[contextStack.back().stackPtr + arg2]) += (int8)arg3;
				break;
			}

			break;

		case sub_stp_const:
			GET_ARG_INT4(arg1, contextStack.back().flow);
			GET_ARG_INT4(arg2, contextStack.back().flow);
			GET_ARG_INT4(arg3, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("sub %d, {%d}, %d", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[contextStack.back().stackPtr + arg2] -= (int1)arg3;
				break;
			case 2:
				*((int2*)&stack[contextStack.back().stackPtr + arg2]) -= (int2)arg3;
				break;
			case 4:
				*((int4*)&stack[contextStack.back().stackPtr + arg2]) -= (int4)arg3;
				break;
			case 8:
				*((int8*)&stack[contextStack.back().stackPtr + arg2]) -= (int8)arg3;
				break;
			}

			break;

		case mul_stp_const:
			GET_ARG_INT4(arg1, contextStack.back().flow);
			GET_ARG_INT4(arg2, contextStack.back().flow);
			GET_ARG_INT4(arg3, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("mul %d, {%d}, %d", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[contextStack.back().stackPtr + arg2] *= (int1)arg3;
				break;
			case 2:
				*((int2*)&stack[contextStack.back().stackPtr + arg2]) *= (int2)arg3;
				break;
			case 4:
				*((int4*)&stack[contextStack.back().stackPtr + arg2]) *= (int4)arg3;
				break;
			case 8:
				*((int8*)&stack[contextStack.back().stackPtr + arg2]) *= (int8)arg3;
				break;
			}

			break;

		case div_stp_const:
			GET_ARG_INT4(arg1, contextStack.back().flow);
			GET_ARG_INT4(arg2, contextStack.back().flow);
			GET_ARG_INT4(arg3, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("div %d, {%d}, %d", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[contextStack.back().stackPtr + arg2] /= (int1)arg3;
				break;
			case 2:
				*((int2*)&stack[contextStack.back().stackPtr + arg2]) /= (int2)arg3;
				break;
			case 4:
				*((int4*)&stack[contextStack.back().stackPtr + arg2]) /= (int4)arg3;
				break;
			case 8:
				*((int8*)&stack[contextStack.back().stackPtr + arg2]) /= (int8)arg3;
				break;
			}

			break;

		case mod_stp_const:
			GET_ARG_INT4(arg1, contextStack.back().flow);
			GET_ARG_INT4(arg2, contextStack.back().flow);
			GET_ARG_INT4(arg3, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("mod %d, {%d}, %d", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[contextStack.back().stackPtr + arg2] %= (int1)arg3;
				break;
			case 2:
				*((int2*)&stack[contextStack.back().stackPtr + arg2]) %= (int2)arg3;
				break;
			case 4:
				*((int4*)&stack[contextStack.back().stackPtr + arg2]) %= (int4)arg3;
				break;
			case 8:
				*((int8*)&stack[contextStack.back().stackPtr + arg2]) %= (int8)arg3;
				break;
			}

			break;

		case not_stp:
			GET_ARG_INT4(arg1, contextStack.back().flow);
			GET_ARG_INT4(arg2, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("not %d, {%d}", arg1, arg2);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[contextStack.back().stackPtr + arg2] = ~stack[contextStack.back().stackPtr + arg2];
				break;
			case 2:
				*((int2*)&stack[contextStack.back().stackPtr + arg2]) = ~(*((int2*)&stack[contextStack.back().stackPtr + arg2]));
				break;
			case 4:
				*((int4*)&stack[contextStack.back().stackPtr + arg2]) = ~(*((int4*)&stack[contextStack.back().stackPtr + arg2]));
				break;
			case 8:
				*((int8*)&stack[contextStack.back().stackPtr + arg2]) = ~(*((int8*)&stack[contextStack.back().stackPtr + arg2]));
				break;
			}

			break;

		case and_stp_stp:
			GET_ARG_INT4(arg1, contextStack.back().flow);
			GET_ARG_INT4(arg2, contextStack.back().flow);
			GET_ARG_INT4(arg3, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("and %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[contextStack.back().stackPtr + arg2] &= stack[contextStack.back().stackPtr + arg3];
				break;
			case 2:
				*((int2*)&stack[contextStack.back().stackPtr + arg2]) &= *((int2*)&stack[contextStack.back().stackPtr + arg3]);
				break;
			case 4:
				*((int4*)&stack[contextStack.back().stackPtr + arg2]) &= *((int4*)&stack[contextStack.back().stackPtr + arg3]);
				break;
			case 8:
				*((int8*)&stack[contextStack.back().stackPtr + arg2]) &= *((int8*)&stack[contextStack.back().stackPtr + arg3]);
				break;
			}

			break;

		case or_stp_stp:
			GET_ARG_INT4(arg1, contextStack.back().flow);
			GET_ARG_INT4(arg2, contextStack.back().flow);
			GET_ARG_INT4(arg3, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("or %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[contextStack.back().stackPtr + arg2] |= stack[contextStack.back().stackPtr + arg3];
				break;
			case 2:
				*((int2*)&stack[contextStack.back().stackPtr + arg2]) |= *((int2*)&stack[contextStack.back().stackPtr + arg3]);
				break;
			case 4:
				*((int4*)&stack[contextStack.back().stackPtr + arg2]) |= *((int4*)&stack[contextStack.back().stackPtr + arg3]);
				break;
			case 8:
				*((int8*)&stack[contextStack.back().stackPtr + arg2]) |= *((int8*)&stack[contextStack.back().stackPtr + arg3]);
				break;
			}

			break;

		case xor_stp_stp:
			GET_ARG_INT4(arg1, contextStack.back().flow);
			GET_ARG_INT4(arg2, contextStack.back().flow);
			GET_ARG_INT4(arg3, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("xor %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[contextStack.back().stackPtr + arg2] ^= stack[contextStack.back().stackPtr + arg3];
				break;
			case 2:
				*((int2*)&stack[contextStack.back().stackPtr + arg2]) ^= *((int2*)&stack[contextStack.back().stackPtr + arg3]);
				break;
			case 4:
				*((int4*)&stack[contextStack.back().stackPtr + arg2]) ^= *((int4*)&stack[contextStack.back().stackPtr + arg3]);
				break;
			case 8:
				*((int8*)&stack[contextStack.back().stackPtr + arg2]) ^= *((int8*)&stack[contextStack.back().stackPtr + arg3]);
				break;
			}

			break;

		case and_stp_const:
			GET_ARG_INT4(arg1, contextStack.back().flow);
			GET_ARG_INT4(arg2, contextStack.back().flow);
			GET_ARG_INT4(arg3, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("and %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[contextStack.back().stackPtr + arg2] &= arg3;
				break;
			case 2:
				*((int2*)&stack[contextStack.back().stackPtr + arg2]) &= (int2)arg3;
				break;
			case 4:
				*((int4*)&stack[contextStack.back().stackPtr + arg2]) &= (int4)arg3;
				break;
			case 8:
				*((int8*)&stack[contextStack.back().stackPtr + arg2]) &= (int8)arg3;
				break;
			}

			break;

		case or_stp_const:
			GET_ARG_INT4(arg1, contextStack.back().flow);
			GET_ARG_INT4(arg2, contextStack.back().flow);
			GET_ARG_INT4(arg3, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("or %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[contextStack.back().stackPtr + arg2] |= arg3;
				break;
			case 2:
				*((int2*)&stack[contextStack.back().stackPtr + arg2]) |= (int2)arg3;
				break;
			case 4:
				*((int4*)&stack[contextStack.back().stackPtr + arg2]) |= (int4)arg3;
				break;
			case 8:
				*((int8*)&stack[contextStack.back().stackPtr + arg2]) |= (int8)arg3;
				break;
			}

			break;

		case xor_stp_const:
			GET_ARG_INT4(arg1, contextStack.back().flow);
			GET_ARG_INT4(arg2, contextStack.back().flow);
			GET_ARG_INT4(arg3, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("xor %d, {%d}, {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				stack[contextStack.back().stackPtr + arg2] ^= arg3;
				break;
			case 2:
				*((int2*)&stack[contextStack.back().stackPtr + arg2]) ^= (int2)arg3;
				break;
			case 4:
				*((int4*)&stack[contextStack.back().stackPtr + arg2]) ^= (int4)arg3;
				break;
			case 8:
				*((int8*)&stack[contextStack.back().stackPtr + arg2]) ^= (int8)arg3;
				break;
			}

			break;

		case if_stp_flow:
			GET_ARG_INT4(arg1, contextStack.back().flow);
			GET_ARG_INT4(arg2, contextStack.back().flow);
			GET_ARG_INT4(arg3, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("if %d, {%d} [%d]", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				if (stack[contextStack.back().stackPtr + arg2] != 0)
				{
					contextStack.back().flow = arg3;
				}
				break;
			case 2:
				if (*((int2*)&stack[contextStack.back().stackPtr + arg2]) != 0)
				{
					contextStack.back().flow = arg3;
				}
				break;
			case 4:
				if (*((int4*)&stack[contextStack.back().stackPtr + arg2]) != 0)
				{
					contextStack.back().flow = arg3;
				}
				break;
			case 8:
				if (*((int8*)&stack[contextStack.back().stackPtr + arg2]) != 0)
				{
					contextStack.back().flow = arg3;
				}
				break;
			}
			break;

		case ifp_stp_flow:
			GET_ARG_INT4(arg1, contextStack.back().flow);
			GET_ARG_INT4(arg2, contextStack.back().flow);
			GET_ARG_INT4(arg3, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("ifp %d, {%d} [%d]", arg1, arg2, arg3);
			fflush(stdout);
#endif
			switch (arg1)
			{
			case 1:
				if (stack[contextStack.back().stackPtr + arg2] > 0)
				{
					contextStack.back().flow = arg3;
				}
				break;
			case 2:
				if (*((int2*)&stack[contextStack.back().stackPtr + arg2]) > 0)
				{
					contextStack.back().flow = arg3;
				}
				break;
			case 4:
				if (*((int4*)&stack[contextStack.back().stackPtr + arg2]) > 0)
				{
					contextStack.back().flow = arg3;
				}
				break;
			case 8:
				if (*((int8*)&stack[contextStack.back().stackPtr + arg2]) > 0)
				{
					contextStack.back().flow = arg3;
				}
				break;
			}
			break;


		case call_stp:
			GET_ARG_INT4(arg1, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("call [{%d}]", arg1);
			fflush(stdout);
#endif
			contextStack.back().stackPtr -= 4;
			*((int4*)&stack[contextStack.back().stackPtr]) = contextStack.back().flow;
			contextStack.back().flow = *((int4*)&stack[contextStack.back().stackPtr + arg1]);

			reportToDebugger(stack, contextStack.back().stackPtr, contextStack.back().stackSize, heap, contextStack.back().heapSize, contextStack.back().flow, fsStepIn);
			break;

		case call_flow:
			GET_ARG_INT4(arg1, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("call [%d]", arg1);
			fflush(stdout);
#endif
			contextStack.back().stackPtr -= 4;
			*((int4*)&stack[contextStack.back().stackPtr]) = contextStack.back().flow;
			contextStack.back().flow = (int4)arg1;

			reportToDebugger(stack, contextStack.back().stackPtr, contextStack.back().stackSize, heap, contextStack.back().heapSize, contextStack.back().flow, fsStepIn);
			break;

		case ret_stp:
#ifdef OUTPUT_INSTRUCTIONS
			printf("ret");
			fflush(stdout);
#endif
			contextStack.back().flow = *((int4*)&stack[contextStack.back().stackPtr]);
			contextStack.back().stackPtr += 4;	// removing the caller's address

			reportToDebugger(stack, contextStack.back().stackPtr, contextStack.back().stackSize, heap, contextStack.back().heapSize, contextStack.back().flow, fsStepOut);
			break;

		case hret_stp:
#ifdef OUTPUT_INSTRUCTIONS
			printf("hret");
			fflush(stdout);
#endif
			// Removing the last context from the context stack
			contextStack.pop_back();

			// Setting the next context from the stack as the current
			stack = &(getMemory()[contextStack.back().stackStart]);
			heap = &(getMemory()[contextStack.back().heapStart]);

			//portHandlingJustFinished = true;
			reportToDebugger(stack, contextStack.back().stackPtr, contextStack.back().stackSize, heap, contextStack.back().heapSize, contextStack.back().flow, fsStepOutHandler);

			break;

		case jmp_flow:
			GET_ARG_INT4(arg1, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("jmp [%d]", arg1);
			fflush(stdout);
#endif
			contextStack.back().flow = arg1;
			break;

		case out_const:
			GET_ARG_INT4(arg1, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("out %d", arg1);
			fflush(stdout);
#endif
			activityContext = contextStack.back();
			activityContext.port = arg1;
			sendMessage();
			break;

		case out_stp:
			GET_ARG_INT4(arg1, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("out {%d}", arg1);
			fflush(stdout);
#endif
			activityContext = contextStack.back();
			activityContext.port = *((int4*)&stack[contextStack.back().stackPtr + arg1]);
			sendMessage();
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
			GET_ARG_INT4(arg1, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("setcont {%d}", arg1);
			fflush(stdout);
#endif
			contextStack.back().readFrom(&stack[contextStack.back().stackPtr + arg1]);
			stack = &(getMemory()[contextStack.back().stackStart]);
			heap = &(getMemory()[contextStack.back().heapStart]);

			break;

		case setcont_m_stp:
			GET_ARG_INT4(arg1, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("setcont [{%d}]", arg1);
			fflush(stdout);
#endif

			tmpAddr = *((int4*)&stack[contextStack.back().stackPtr + arg1]);
			contextStack.back().readFrom(((int1*)&heap[tmpAddr]));
			stack = &(getMemory()[contextStack.back().stackStart]);
			heap = &(getMemory()[contextStack.back().heapStart]);

			break;

		case getcont_stp:
			GET_ARG_INT4(arg1, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("getcont {%d}", arg1);
			fflush(stdout);
#endif

			contextStack.back().writeTo(&stack[contextStack.back().stackPtr + arg1]);

			break;

		case getcont_m_stp:
			GET_ARG_INT4(arg1, contextStack.back().flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("getcont [{%d}]", arg1);
			fflush(stdout);
#endif

			tmpAddr = *((int4*)&stack[contextStack.back().stackPtr + arg1]);
			contextStack.back().writeTo(((int1*)&heap[tmpAddr]));

			break;

		}

#ifdef OUTPUT_INSTRUCTIONS
		printf("\t\t\t{");
		if (contextStack.back().stackPtr < contextStack.back().stackSize)  printf("%d", stack[contextStack.back().stackPtr]);
		for (unsigned int i = contextStack.back().stackPtr + 1; i < contextStack.back().stackSize; i++)
			printf(", %d", stack[i]);
		printf("}\n");
		fflush(stdout);
#endif
		return true;
//	}
}
