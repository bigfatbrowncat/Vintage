#include "Debugger.h"

Debugger::Debugger(FILE* debug_symbols, SDLScreen& screen) :
	screen(screen), running(false), haltPending(false), stepPending(false)
{
	pthread_mutex_init(&printing_mutex, NULL);

	while (!feof(debug_symbols))
	{
		int4 mem_pos;
		char read_char;
		wstring lines = L"";

		fread(&mem_pos, sizeof(mem_pos), 1, debug_symbols);
		bool first_char = true;
		do
		{
			fread(&read_char, 1, 1, debug_symbols);
			if (read_char != 0 && (read_char != '\n' || first_char == false) && read_char != '\r') 	// We remove first caret return
			{
				lines += read_char;
			}
			if (read_char != '\n' && read_char != '\r') first_char = false;
		}
		while (read_char != 0);

		DebugEntry newEntry;
		newEntry.mem_pos = mem_pos;
		newEntry.lines = lines;
		entries.push_back(newEntry);
	}
}

Debugger::~Debugger()
{
	pthread_mutex_destroy(&printing_mutex);
}

const DebugEntry* Debugger::findLine(int4 mem_pos) const
{
	for (unsigned int i = 0; i < entries.size() - 1; i++)
	{
		if (entries[i + 1].mem_pos > mem_pos &&
			entries[i].mem_pos <= mem_pos)
		{
			return &(entries[i]);
		}
	}
	return &(entries[entries.size() - 1]);
}

void Debugger::printMenu()
{
	screen.SetCursorPosition(0, 0);
	screen.SelectForeColor(0, 0, 0);

	screen.SelectBackColor(192, 192, 192);
	screen.Write(L" Vintage hardware debugging tool ");
	screen.SelectBackColor(0, 0, 0);
	screen.Write(L"   ");
	if (this->running)
	{
		screen.SelectBackColor(192, 192, 192);
	}
	else
	{
		screen.SelectBackColor(128, 128, 128);
	}
	screen.Write(L" F1 Run ");
	screen.SelectBackColor(0, 0, 0);
	screen.Write(L" ");
	if (!this->running && !this->stepPending)
	{
		screen.SelectBackColor(192, 192, 192);
	}
	else
	{
		screen.SelectBackColor(128, 128, 128);
	}
	screen.Write(L" F2 Pause ");
	screen.SelectBackColor(0, 0, 0);
	screen.Write(L" ");
	if (this->stepPending)
	{
		screen.SelectBackColor(192, 192, 192);
	}
	else
	{
		screen.SelectBackColor(128, 128, 128);
	}
	screen.Write(L" F3 Step ");
	screen.SelectBackColor(0, 0, 0);
	screen.Write(L" ");
	screen.SelectBackColor(128, 128, 128);
	screen.Write(L" F4 Halt CPU ");
	screen.SelectBackColor(0, 0, 0);
	screen.Write(L" ");
	screen.SetCursorPosition(0, screen.getFrameBufferHeight() - 1);
}

void Debugger::stepDone(int4 flow)
{
	pthread_mutex_lock(&printing_mutex);
	screen.SetCursorPosition(0, screen.getFrameBufferHeight() - 1);

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

	printMenu();
	pthread_mutex_unlock(&printing_mutex);
}
