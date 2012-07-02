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

using namespace std;

#define CODE_LINE_MAX_LENGTH		256

class Screen;
class DebuggerKeyboardController;

enum DebuggerOrder
{
	doWait,
	doGo,
	doHalt
};

struct DebugEntry
{
	int4 mem_pos;
	wstring lines;
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

enum FlowState
{
	fsLinear,
	fsStepIn,
	fsStepOut
};

class Debugger
{
	friend class DebuggerKeyboardController;
private:
	int topSpace;

	int wStackTopRow;
	int wHeapTopRow;
	int wStackBytes;
	int wHeapBytes;

	DebuggerActiveWindow activeWindow;
	vector<DebugEntry> entries;
	SDLScreen& screen;
	volatile bool running;
	volatile bool haltPending;
	volatile bool stepOverPending;
	volatile bool stepIntoPending;
	volatile bool stepOutPending;

	volatile bool runningOut;
	volatile bool runningOver;

	pthread_mutex_t printingMutex;

	int4 flow;
	int1* stack;
	int4 stackAllocatedSize;
	int4 stackMaxSize;
	int1* heap;
	int4 heapSize;

	int flowLevel;
	int savedFlowLevel;

protected:
	void printMenu();
	void printFixed(int x, int y, const wchar_t* str, int length);
public:
	void updateUI();
	void handleControlKey(ControlKey ck)
	{
		pthread_mutex_lock(&printingMutex);
		int heapLines = heapSize / wHeapBytes;
		int stackLines = stackMaxSize / wStackBytes;

		if (ck == ckTab)
		{
			activeWindow = (DebuggerActiveWindow)((activeWindow + 1) % dawSize);
		}

		else if (ck == ckUp && activeWindow == dawHeap)
		{
			wHeapTopRow --;
		}
		else if (ck == ckDown && activeWindow == dawHeap)
		{
			wHeapTopRow ++;
		}
		else if (ck == ckUp && activeWindow == dawStack)
		{
			wStackTopRow --;
		}
		else if (ck == ckDown && activeWindow == dawStack)
		{
			wStackTopRow ++;
		}

		else if (ck == ckPageUp && activeWindow == dawHeap)
		{
			wHeapTopRow -= topSpace / 2;
		}
		else if (ck == ckPageDown && activeWindow == dawHeap)
		{
			wHeapTopRow += topSpace / 2;
		}
		else if (ck == ckPageUp && activeWindow == dawStack)
		{
			wStackTopRow -= topSpace / 2;
		}
		else if (ck == ckPageDown && activeWindow == dawStack)
		{
			wStackTopRow += topSpace / 2;
		}

		if (wHeapTopRow < 0) wHeapTopRow = 0;
		if (wStackTopRow < 0) wStackTopRow = 0;
		if (wHeapTopRow > heapLines) wHeapTopRow = heapLines;
		if (wStackTopRow > stackLines) wStackTopRow = stackLines;
		pthread_mutex_unlock(&printingMutex);

		updateUI();
	}

	const vector<DebugEntry>& getEntries() const { return entries; }
	Debugger(FILE* debug_symbols, SDLScreen& screen);
	virtual ~Debugger();
	int findLine(int4 mem_pos) const;
	void flowChanged(int4 flow, int1* stack, int4 stackMaxSize, int4 stackSize, int1* heap, int4 heapSize);

	const DebuggerOrder askForOrder(FlowState flowState)
	{
		pthread_mutex_lock(&printingMutex);
		if (flowState == fsStepIn)
			flowLevel ++;
		if (flowState == fsStepOut)
			flowLevel --;

		DebuggerOrder res;
		if (haltPending)
		{
			res = doHalt;
		}
		else if (!running)
		{
			if (stepIntoPending)
			{
				stepIntoPending = false;
				res = doGo;
			}
			else if (stepOutPending)
			{
				runningOut = true;
				stepOutPending = false;
				savedFlowLevel = flowLevel;
				res = doGo;
			}
			else if (runningOut)
			{
				if (flowLevel < savedFlowLevel)
				{
					runningOut = false;
					res = doWait;
				}
				else
				{
					res = doGo;
				}
			}
			else if (stepOverPending)
			{
				runningOver = true;
				stepOverPending = false;
				savedFlowLevel = flowLevel;
				res = doGo;
			}
			else if (runningOver)
			{
				if (flowLevel == savedFlowLevel)
				{
					runningOver = false;
					res = doWait;
				}
				else
				{
					res = doGo;
				}
			}
			else
			{
				res = doWait;
			}
		}
		else
		{
			res = doGo;
		}
		pthread_mutex_unlock(&printingMutex);
		return res;
	}

	void run()
	{
		if (!running && !runningOut && !runningOver)
		{
			pthread_mutex_lock(&printingMutex);
			running = true;
			printMenu();
			pthread_mutex_unlock(&printingMutex);
		}
	}
	void stop()
	{
		if (running)
		{
			pthread_mutex_lock(&printingMutex);
			running = false;
			printMenu();
			pthread_mutex_unlock(&printingMutex);
		}
	}
	void stepOver()
	{
		if (!runningOut && !runningOver)
		{
			pthread_mutex_lock(&printingMutex);
			running = false;
			stepOverPending = true;
			printMenu();
			pthread_mutex_unlock(&printingMutex);
		}
	}
	void stepInto()
	{
		if (!runningOut && !runningOver)
		{
			pthread_mutex_lock(&printingMutex);
			running = false;
			stepIntoPending = true;
			printMenu();
			pthread_mutex_unlock(&printingMutex);
		}
	}
	void stepOut()
	{
		if (!runningOut && !runningOver)
		{
			pthread_mutex_lock(&printingMutex);
			running = false;
			stepOutPending = true;
			printMenu();
			pthread_mutex_unlock(&printingMutex);
		}
	}
	void halt()
	{
		pthread_mutex_lock(&printingMutex);
		haltPending = true;
		printMenu();
		pthread_mutex_unlock(&printingMutex);
	}
};

#endif
