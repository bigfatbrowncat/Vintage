#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <wchar.h>

#include "SDLConsole.h"
#include "Debugger.h"
#include "CPU.h"
#include "../../FontEditor/include/Font.h"

#include <string>

using namespace std;

#define	RESULT_OK									0
#define	RESULT_ERROR								1

int main(int argc, char* argv[])
{
	CPU cpu(1024 * 1024, 1024 * 1024, 256, 64);			// 1MB heap and 1MB stack

	Keyboard kbd(cpu, 2, 256);

	Font font("res/font.txt");
	Font curfont("res/curfont.txt");
	SDLTerminal window(kbd, font, curfont);

	HardwareTimer ht(cpu, 3);
	Console term(cpu, 1, &(window.getScreen(0)));

	Debugger* dbg = NULL;

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
			}
		}

		binfile = fopen(binary_name, "rb");
		if (binfile != NULL)
		{
			printf("VM loader error: can't open the binary file\n");

			int4 act_read;
			act_read = fread(cpu.GetHeap(), 1, cpu.GetHeapSize(), binfile);
			printf("Loaded %d bytes binary\n", act_read);

			if (dbg_symbols_file != NULL)
			{
				dbg = new Debugger(dbg_symbols_file, window.getScreen(1));
				cpu.setDebugger(*dbg);

				printf("Loaded debug symbols and source code\n");
			}
		}
	}

	term.TurnOn();
	ht.TurnOn();
	kbd.TurnOn();
	cpu.TurnOn();

	window.Run();

	cpu.TurnOff();
	kbd.TurnOff();
	ht.TurnOff();
	term.TurnOff();

exit:
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
