#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <wchar.h>

#include "SDLTerminal.h"
#include "SDLScreen.h"
#include "Debugger.h"
#include "CPU.h"
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
	if (cpu->isTerminated())
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
	int4 heapSize = 1024 * 1024;										// 1MB
	int4 stackSize = 1024 * 1024;										// 1MB

	CPU cpu(heapSize + stackSize, 0, heapSize, heapSize, stackSize, 256, 64);		// 256 ports with buffer length of 64

	CachedFont font("res/font.txt");
	CachedFont curfont("res/curfont.txt");
	SDLTerminal terminal(font, curfont);
	terminal.setCustomEventsHandler(handleTerminalCustomEvents, &cpu);

	SDLScreen& cpuScreen = terminal.getScreen(0);			// Ctrl + F1
	SDLScreen& debuggerScreen = terminal.getScreen(1);		// Ctrl + F2


	HardwareTimer ht(cpu, 1);							// Hardware timer on port 1 -- the highest priority
	Console cpuConsole(cpu, 2, &cpuScreen);				// Terminal on port 2
	CPUKeyboardController kbd(cpu, 3, 256);				// Keyboard on the port 3

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
			act_read = fread(cpu.GetMemory(), 1, cpu.GetMemorySize(), binfile);
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

	cpuConsole.TurnOn();
	ht.TurnOn();
	kbd.TurnOn();
	cpu.TurnOn();

	terminal.Run();

	cpu.TurnOff();
	kbd.TurnOff();
	ht.TurnOff();
	cpuConsole.TurnOff();

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
