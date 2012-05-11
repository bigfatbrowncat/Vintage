#include <stdio.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include <iomanip>

#include "instructions.h"
#include "Debugger.h"
#include "CPU.h"

#define GET_ARG(arg, flow)			arg = *((int4*)(&heap[flow])); flow += sizeof(int4);

//#define OUTPUT_INSTRUCTIONS

void Debugger::reportFlow(int4 flow)
{
	const DebugEntry* fl = findLine(flow);

	screen.SelectForeColor(128, 128, 128);
	wstringstream strs;
	strs << std::setw(8) << setfill(L'0') << std::hex << uppercase << fl->mem_pos << L" ";
	screen.Write(strs.str().c_str());

	screen.SelectForeColor(192, 192, 192);
	wstringstream strs2;
	if (fl != NULL)
	{
		//printf("0x%X: %s\n", fl->mem_pos, fl->lines.c_str());
		strs2 << fl->lines << L"\n";
	}
	else
	{
		//printf("0x%X: <<No such address in debug symbols>>\n");
		wstringstream strs2;
		strs2 << L"<< No such address in debug symbols >>\n";
	}
	screen.Write(strs2.str().c_str());
}


void* CPU_activity_function(void* arg)
{
	((CPU*)arg)->ActivityFunction();
	return NULL;
}

void CPU::TurnOn()
{
	terminationPending = false;
	pthread_create(&this->activity, NULL, &CPU_activity_function, this);
}

void CPU::ActivityFunction()
{
	int4 flow = 0;
	int4 addr;

	while (!terminationPending)
	{
		// Logging to debug
		if (pDebugger != NULL)
		{
			pDebugger->reportFlow(flow);
		}

		// Checking ports input
		pthread_mutex_lock(&portReadingMutex);
		if (anyPortInWaiting && !portInHandlingInProgress)
		{
			int ports_waiting = 0;
			int port = -1;
			for (int i = 0; i < ports_count; i++)
			{
				if (portInWaiting[i] && portInHandlers[i] > 0)
				{
					ports_waiting++;
					if (port == -1) port = i;
				}
			}

			// Calling the handler
			if (port >= 0)		// it's better to use a flag here
			{
				portInHandlingInProgress = true;
				portInHandling = port;
				portInWaiting[port] = false;

				stackPtr -= 4;
				*((int4*)&stack[stackPtr]) = flow;
				flow = portInHandlers[portInHandling];

				stackPtr -= portInWaitingDataLength[portInHandling];

				for (int i = 0; i < portInWaitingDataLength[portInHandling]; i++)
				{
					*((int1*)&stack[stackPtr + i]) = *((int1*)&portInWaitingData[port * port_data_length + i]);
				}

				if (ports_waiting == 1) anyPortInWaiting = false;
			}

		}
		pthread_mutex_unlock(&portReadingMutex);

#ifdef OUTPUT_INSTRUCTIONS
		printf("%d:\t", flow);
#endif
		int4 arg1, arg2, arg3;

		// Parsing
		instr_t instr = heap[flow];

		flow++;

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
			GET_ARG(arg1, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("alloc %d", arg1);
			fflush(stdout);
#endif
			stackPtr -= arg1;
			break;

		case free_const:
			GET_ARG(arg1, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("free %d", arg1);
			fflush(stdout);
#endif
			stackPtr += arg1;
			break;

		case mov_stp_stp:
			GET_ARG(arg1, flow);
			GET_ARG(arg2, flow);
			GET_ARG(arg3, flow);
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
			GET_ARG(arg1, flow);
			GET_ARG(arg2, flow);
			GET_ARG(arg3, flow);
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
			GET_ARG(arg1, flow);
			GET_ARG(arg2, flow);
			GET_ARG(arg3, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("mov %d, [{%d}], {%d}", arg1, arg2, arg3);
			fflush(stdout);
#endif
			addr = *((int4*)&stack[stackPtr + arg2]);

			switch (arg1)
			{
			case 1:
				heap[addr] = stack[stackPtr + arg2];
				break;
			case 2:
				*((int2*)&heap[addr]) = *((int2*)&stack[stackPtr + arg3]);
				break;
			case 4:
				*((int4*)&heap[addr]) = *((int4*)&stack[stackPtr + arg3]);
				break;
			case 8:
				*((int8*)&heap[addr]) = *((int8*)&stack[stackPtr + arg3]);
				break;
			}

			break;

		case mov_stp_m_stp:
			GET_ARG(arg1, flow);
			GET_ARG(arg2, flow);
			GET_ARG(arg3, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("mov %d, {%d}, [{%d}]", arg1, arg2, arg3);
			fflush(stdout);
#endif
			addr = *((int4*)&stack[stackPtr + arg3]);

			switch (arg1)
			{
			case 1:
				stack[stackPtr - arg2] = heap[addr];
				break;
			case 2:
				*((int2*)&stack[stackPtr + arg2]) = *((int2*)&heap[addr]);
				break;
			case 4:
				*((int4*)&stack[stackPtr + arg2]) = *((int4*)&heap[addr]);
				break;
			case 8:
				*((int8*)&stack[stackPtr + arg2]) = *((int8*)&heap[addr]);
				break;
			}

			break;


		case add_stp_stp:
			GET_ARG(arg1, flow);
			GET_ARG(arg2, flow);
			GET_ARG(arg3, flow);
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
			GET_ARG(arg1, flow);
			GET_ARG(arg2, flow);
			GET_ARG(arg3, flow);
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
			GET_ARG(arg1, flow);
			GET_ARG(arg2, flow);
			GET_ARG(arg3, flow);
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
			GET_ARG(arg1, flow);
			GET_ARG(arg2, flow);
			GET_ARG(arg3, flow);
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
			GET_ARG(arg1, flow);
			GET_ARG(arg2, flow);
			GET_ARG(arg3, flow);
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
			GET_ARG(arg1, flow);
			GET_ARG(arg2, flow);
			GET_ARG(arg3, flow);
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
			GET_ARG(arg1, flow);
			GET_ARG(arg2, flow);
			GET_ARG(arg3, flow);
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
			GET_ARG(arg1, flow);
			GET_ARG(arg2, flow);
			GET_ARG(arg3, flow);
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
			GET_ARG(arg1, flow);
			GET_ARG(arg2, flow);
			GET_ARG(arg3, flow);
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
			GET_ARG(arg1, flow);
			GET_ARG(arg2, flow);
			GET_ARG(arg3, flow);
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
			GET_ARG(arg1, flow);
			GET_ARG(arg2, flow);
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
			GET_ARG(arg1, flow);
			GET_ARG(arg2, flow);
			GET_ARG(arg3, flow);
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
			GET_ARG(arg1, flow);
			GET_ARG(arg2, flow);
			GET_ARG(arg3, flow);
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
			GET_ARG(arg1, flow);
			GET_ARG(arg2, flow);
			GET_ARG(arg3, flow);
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
			GET_ARG(arg1, flow);
			GET_ARG(arg2, flow);
			GET_ARG(arg3, flow);
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
			GET_ARG(arg1, flow);
			GET_ARG(arg2, flow);
			GET_ARG(arg3, flow);
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
			GET_ARG(arg1, flow);
			GET_ARG(arg2, flow);
			GET_ARG(arg3, flow);
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
			GET_ARG(arg1, flow);
			GET_ARG(arg2, flow);
			GET_ARG(arg3, flow);
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
			GET_ARG(arg1, flow);
			GET_ARG(arg2, flow);
			GET_ARG(arg3, flow);
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
			GET_ARG(arg1, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("call [{%d}]", arg1);
			fflush(stdout);
#endif
			stackPtr -= 4;
			*((int4*)&stack[stackPtr]) = flow;
			flow = *((int4*)&stack[stackPtr + arg1]);
			break;

		case call_flow:
			GET_ARG(arg1, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("call [%d]", arg1);
			fflush(stdout);
#endif
			stackPtr -= 4;
			*((int4*)&stack[stackPtr]) = flow;
			flow = (int4)arg1;
			break;

		case ret_stp:
#ifdef OUTPUT_INSTRUCTIONS
			printf("ret");
			fflush(stdout);
#endif
			flow = *((int4*)&stack[stackPtr]);
			stackPtr += 4;	// removing the callr's address
			break;

		case hret_stp:
#ifdef OUTPUT_INSTRUCTIONS
			printf("hret");
			fflush(stdout);
#endif
			pthread_mutex_lock(&portReadingMutex);
			stackPtr += portInWaitingDataLength[portInHandling];	// removing port data
			flow = *((int4*)&stack[stackPtr]);
			stackPtr += 4;	// removing the callr's address
			portInHandlingInProgress = false;
			pthread_mutex_unlock(&portReadingMutex);
			break;

		case jmp_flow:
			GET_ARG(arg1, flow);
#ifdef OUTPUT_INSTRUCTIONS
			//printf("jmp [%d]", arg1);
			fflush(stdout);
#endif
			flow = arg1;
			break;

		case out_const:
			GET_ARG(arg1, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("out %d", arg1);
			fflush(stdout);
#endif
			devices[arg1]->CallHandler(heap, stack, stackPtr);
			break;

		case regin_const_flow:
			GET_ARG(arg1, flow);
			GET_ARG(arg2, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("regin %d, %d", arg1, arg2);
			fflush(stdout);
#endif
			portInHandlers[arg1] = arg2;
			break;

		case uregin_const:
			GET_ARG(arg1, flow);
			GET_ARG(arg2, flow);
#ifdef OUTPUT_INSTRUCTIONS
			printf("uregin %d", arg1);
			fflush(stdout);
#endif
			portInHandlers[arg1] = 0;
			break;

		case halt:
#ifdef OUTPUT_INSTRUCTIONS
			printf("halt");
			fflush(stdout);
#endif
			// Halt
			return;		// Close the world...
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

}
