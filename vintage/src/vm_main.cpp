#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <wchar.h>

#include "SDLTerminal.h"
#include "SDLScreen.h"
#include "Debugger.h"
#include "CPU.h"
#include "MessageContext.h"
#include "../../FontEditor/include/Font.h"
#include "HardwareTimer.h"
#include "Console.h"
#include "CPUKeyboardController.h"
#include "DebuggerKeyboardController.h"

#include <string>

using namespace std;

#define	RESULT_OK									0
#define	RESULT_ERROR								1

bool handleTerminalCustomEvents(void* data)
{
	CPU* cpu = (CPU*)data;
	if (cpu->getState() == hdsOff)
	{
		return false;
	}
	else
	{
		return true;
	}
}

int main(int argc, char* argv[])
{
	int4 portsCount = 256;												// 256 ports with buffer length of 64

	int4 initialHeapSize = 1024 * 1024;									// 1MB
	int4 initialStackSize = 1024 * 1024;								// 1MB
	int4 memorySize = initialHeapSize + initialStackSize;

	int1* memory = new int1[memorySize];

	// (int4 heapStart, int4 heapSize, int4 stackStart, int4 stackSize, int4 stackPtr, int4 flow)

	MessageContext initialContext(0, 0, initialHeapSize, initialHeapSize, initialStackSize, initialStackSize, 0);
	CPU cpu(portsCount, memory, memorySize, initialContext);

	CachedFont font("res/font.txt");
	CachedFont curfont("res/curfont.txt");
	SDLTerminal terminal(font, curfont);
	terminal.setCustomEventsHandler(handleTerminalCustomEvents, &cpu);

	SDLScreen& cpuScreen = terminal.getScreen(0);			// Ctrl + F1
	SDLScreen& debuggerScreen = terminal.getScreen(1);		// Ctrl + F2

	HardwareTimer hardTimer(memory, memorySize);
	Console cpuConsole(&cpuScreen, memory, memorySize);
	CPUKeyboardController kbd(256, memory, memorySize);

	HardwareDevice::connectDevices(hardTimer, 1, cpu, 1);		// Hardware timer on port 1 -- the highest priority
	HardwareDevice::connectDevices(cpuConsole, 1, cpu, 2);		// Console on port 2
	HardwareDevice::connectDevices(kbd, 1, cpu, 3);				// Keyboard on the port 3

	cpuScreen.setKeyboardController(&kbd);

	Debugger* dbg = NULL;
	DebuggerKeyboardController* dbg_kbd = NULL;

	FILE *binfile = NULL, *dbg_symbols_file = NULL;

	if (argc >= 2)	// Binary name is present
	{
		char* binary_name = argv[1];
		char* dbg_symbols_name = NULL;

		if (argc > 2)
		{
			dbg_symbols_name = argv[2];
			dbg_symbols_file = fopen(dbg_symbols_name, "rb");
			if (dbg_symbols_file == NULL)
			{
				printf("VM loader error: can't open the debugging symbols file\n");
				return 1;
			}
		}

		binfile = fopen(binary_name, "rb");
		if (binfile != NULL)
		{
			printf("Loading the binary...\n");

			int4 act_read;
			act_read = fread(cpu.getMemory(), 1, cpu.getMemorySize(), binfile);
			printf("Loaded %d bytes.\n", act_read);

			if (dbg_symbols_file != NULL)
			{
				dbg = new Debugger(dbg_symbols_file, debuggerScreen);
				dbg_kbd = new DebuggerKeyboardController(*dbg);
				debuggerScreen.setKeyboardController(dbg_kbd);
				cpu.setDebugger(*dbg);

				printf("Debug symbols loaded\n");
			}
		}
		else
		{
			printf("VM loader error: can't open the binary file\n");
			return 2;
		}
	}

	cpuConsole.turnOn();
	hardTimer.turnOn();
	kbd.turnOn();
	cpu.turnOn();

	try
	{
		terminal.Run();
	}
	catch(int e)
	{
		printf("exc: %x\n", e);
	}
	GLenum err = glGetError();

	cpu.turnOff();
	kbd.turnOff();
	hardTimer.turnOff();
	cpuConsole.turnOff();

	if (dbg != NULL)
	{
		printf("Destroying the debugger\n");
		delete dbg;
	}

	if (binfile != NULL) fclose(binfile);
	if (dbg_symbols_file != NULL) fclose(dbg_symbols_file);

    printf("Bye!\n");
	return RESULT_OK;
}
