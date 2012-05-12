class Debugger;

#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <map>
#include <stdio.h>

#include "SDLScreen.h"

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
	Debugger(FILE* debug_symbols, SDLScreen& screen);
	const DebugEntry* findLine(int4 mem_pos) const;
	void reportFlow(int4 flow);
};

#endif
