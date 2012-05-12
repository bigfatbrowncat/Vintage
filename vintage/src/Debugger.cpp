#include "Debugger.h"

Debugger::Debugger(FILE* debug_symbols, SDLScreen& screen) :
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
