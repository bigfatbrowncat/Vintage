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

int Debugger::findLine(int4 mem_pos) const
{
	for (unsigned int i = 0; i < entries.size() - 1; i++)
	{
		if (entries[i + 1].mem_pos > mem_pos &&
			entries[i].mem_pos <= mem_pos)
		{
			return i;
		}
	}
	return entries.size();
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

void Debugger::writeFixed(const wchar_t* str, int length)
{
	bool eoln = false;
	for (int i = 0; i < length; i++)
	{
		if (!eoln && (str[i] == 0 || str[i] == L'\r' || str[i] == L'\n'))
		{
			eoln = true;
		}

		if (eoln)
		{
			screen.Write(L" ");
		}
		else
		{
			wchar_t chr2[2];
			chr2[0] = str[i];
			chr2[1] = 0;

			screen.Write(chr2);
		}
	}

}

void Debugger::stepDone(int4 flow)
{
	pthread_mutex_lock(&printing_mutex);
	//screen.SetCursorPosition(0, screen.getFrameBufferHeight() - 1);

	for (int i = -18; i <= 18; i++)
	{
		int index = findLine(flow) + i;

		screen.SetCursorPosition(0, screen.getFrameBufferHeight() - 20 + i);

		if (i != 0)
		{
			screen.SelectForeColor(128, 128, 128);
			screen.SelectBackColor(0, 0, 0);
		}
		else
		{
			screen.SelectForeColor(0, 0, 0);
			screen.SelectBackColor(128, 128, 128);
		}

		wstringstream strs;
		if (index >= 0 && index < (int)entries.size())
		{
			strs << std::setw(8) << setfill(L'0') << std::hex << uppercase << entries[index].mem_pos << L" ";
		}
		else
		{
			strs << L"         ";
		}
		writeFixed(strs.str().c_str(), 9);

		if (i != 0)
		{
			screen.SelectForeColor(192, 192, 192);
			screen.SelectBackColor(0, 0, 0);
		}
		else
		{
			screen.SelectForeColor(0, 0, 0);
			screen.SelectBackColor(128, 128, 128);
		}

		wstringstream strs2;
		if (index >= 0 && index < (int)entries.size())
		{
			//printf("0x%X: %s\n", fl->mem_pos, fl->lines.c_str());
			strs2 << entries[index].lines << L"\n";
		}
		else
		{
			//printf("0x%X: <<No such address in debug symbols>>\n");
			wstringstream strs2;
			strs2 << L"<< No such address in debug symbols >>\n";
		}
		writeFixed(strs2.str().c_str(), screen.getFrameBufferWidth() - 9);
	}

	printMenu();
	pthread_mutex_unlock(&printing_mutex);
}
