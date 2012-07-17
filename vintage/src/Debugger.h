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
	ckTab,
	ckSpace
};

enum DebuggerState
{
	dsRunningPending,
	dsStepIntoPending,
	dsStepOutPending,
	dsStepOverPending,
	dsHaltPending,
	dsRunning,
	dsRunningInto,
	dsRunningOut,
	dsRunningOver,
	dsStopped
};

enum FlowLayerType { fltNormal, fltHandler };

struct FlowLayer
{
	FlowLayerType type;
	int4 lastFlow;
	FlowLayer(FlowLayerType type = fltNormal, int4 lastFlow = 0)
		: type(type), lastFlow(lastFlow)
	{

	}

	bool operator == (FlowLayer other)
	{
		return type == other.type && lastFlow == other.lastFlow;
	}

	bool operator != (FlowLayer other)
	{
		return type != other.type || lastFlow != other.lastFlow;
	}
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

	volatile DebuggerState state;

	vector<FlowLayer> flowLayers;
	FlowLayer savedFlowLayer;

	bool savedFlowLayerExists()
	{
		for (vector<FlowLayer>::iterator iter = flowLayers.begin(); iter != flowLayers.end(); iter++)
		{
			if (savedFlowLayer == *iter)
			{
				return true;
			}
		}
		return false;
	}

	volatile int4 wSelectedLine;
	volatile bool wSelectedLineFollowsFlow;

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
	void updateUI();
	void handleControlKey(ControlKey ck);

	const vector<DebuggingSymbolsEntry>& getEntries() const { return entries; }
	Debugger(FILE* debug_symbols, SDLScreen& screen);
	virtual ~Debugger();
	int findLine(int4 mem_pos) const;
	int lastLine() const { return entries.size() - 1; }
	void reportFlowStateChanged(FlowState flowState);
	void reportCPUState(int4 flow, int1* stack, int4 stackMaxSize, int4 stackSize, int1* heap, int4 heapSize);

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
