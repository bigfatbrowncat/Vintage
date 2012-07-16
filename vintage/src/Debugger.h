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
#include "HardwareDevice.h"
#include "Debugger.h"
#include "FlowState.h"
#include "DebuggerKeyboardController.h"

using namespace std;

enum DebuggerOrder
{
	doWait,
	doGo,
	doHalt
};

struct DebuggingSymbolsEntry
{
	int4 memPos;
	wstring codeLine;
};

struct Breakpoint
{
	int4 memPos;
	Breakpoint(int4 memPos) : memPos(memPos) {}
};

enum DebuggerActiveWindow
{
	dawCode		= 0,
	dawStack	= 1,
	dawHeap		= 2,
	dawSize		= 3
};

enum ControlKey
{
	ckUp,
	ckDown,
	ckPageUp,
	ckPageDown,
	ckLeft,
	ckRight,
	ckTab
};

class Debugger
{
	friend class DebuggerKeyboardController;
private:
	int topSpace;

	int wHeapBytes;
	int wHeapTopRow;
	int wStackTopRow;
	int wStackBytes;

	DebuggerActiveWindow activeWindow;
	vector<DebuggingSymbolsEntry> entries;
	vector<Breakpoint> breakpoints;
	SDLScreen& screen;

	volatile bool running;
	volatile bool runningPending;
	volatile bool runningOut;
	volatile bool runningOver;
	volatile bool haltPending;
	volatile bool stepOverPending;
	volatile bool stepIntoPending;
	volatile bool stepOutPending;

	volatile int flowLevel;
	volatile int savedFlowLevel;

	volatile int4 wSelectedLine;

	pthread_mutex_t printingMutex;

	int4 flow;
	int1* stack;
	int4 stackAllocatedSize;
	int4 stackMaxSize;
	int1* heap;
	int4 heapSize;


protected:
	void printMenu();
	void printFixed(int x, int y, const wchar_t* str, int length);
	vector<Breakpoint>::iterator findBreakpointAt(int4 memPos);
public:
	bool isRunning() { return running; }
	void updateUI();
	void handleControlKey(ControlKey ck);

	const vector<DebuggingSymbolsEntry>& getEntries() const { return entries; }
	Debugger(FILE* debug_symbols, SDLScreen& screen);
	virtual ~Debugger();
	int findLine(int4 mem_pos) const;
	int lastLine() const { return entries.size() - 1; }
	void reportFlowStateEvent(FlowState flowState);
	void flowChanged(int4 flow, int1* stack, int4 stackMaxSize, int4 stackSize, int1* heap, int4 heapSize);

	const DebuggerOrder askForOrder();
	void toggleBreakpointAt(int4 memPos);

	void run();
	void stop();
	void stepOver();
	void stepInto();
	void stepOut();
	void halt();
	void toggleBreakpoint();
};

#endif
