#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <vector>
#include <string>
#include <map>
#include <stdio.h>

using namespace std;

#define CODE_LINE_MAX_LENGTH		256

class SDLScreen;

struct DebugEntry
{
	int4 mem_pos;
	wstring lines;
};

class Debugger
{
private:
	vector<DebugEntry> entries;
	SDLScreen& screen;
public:
	Debugger(FILE* debug_symbols, SDLScreen& screen) :
		screen(screen)
	{
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
	const DebugEntry* findLine(int4 mem_pos) const
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
	void reportFlow(int4 flow);
};

#endif
