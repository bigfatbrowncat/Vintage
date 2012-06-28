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
	Wait,
	Go,
	Halt
};

struct DebugEntry
{
	int4 mem_pos;
	wstring lines;
};

struct WatchEntry
{
	int4 stack_pos;
	int1 length;
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

	int wStackTopRow;
	int wHeapTopRow;
	int wStackBytes;
	int wHeapBytes;


	DebuggerActiveWindow activeWindow;
	vector<DebugEntry> entries;
	SDLScreen& screen;
	volatile bool running;
	volatile bool haltPending;
	volatile bool stepPending;
	pthread_mutex_t printingMutex;

	int4 flow;
	int1* stack;
	int4 stackSize;
	int4 stackMaxSize;
	int1* heap;
	int4 heapSize;

protected:
	void printMenu();
	void printFixed(int x, int y, const wchar_t* str, int length);
public:
	void updateUI();
	void handleControlKey(ControlKey ck)
	{
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

		updateUI();
	}

	const vector<DebugEntry>& getEntries() const { return entries; }
	Debugger(FILE* debug_symbols, SDLScreen& screen);
	virtual ~Debugger();
	int findLine(int4 mem_pos) const;
	void flowChanged(int4 flow, int1* stack, int4 stackMaxSize, int4 stackSize, int1* heap, int4 heapSize);
	const DebuggerOrder askForOrder(int4 flow)
	{
		if (haltPending)
		{
			return Halt;
		}
		else if (!running)
		{
			if (stepPending)
			{
				stepPending = false;
				return Go;
			}
			else
			{
				return Wait;
			}
		}
		else
		{
			return Go;
		}
	}
	void run()
	{
		pthread_mutex_lock(&printingMutex);
		running = true;
		printMenu();
		pthread_mutex_unlock(&printingMutex);
	}
	void stop()
	{
		pthread_mutex_lock(&printingMutex);
		running = false;
		printMenu();
		pthread_mutex_unlock(&printingMutex);
	}
	void step()
	{
		pthread_mutex_lock(&printingMutex);
		running = false;
		stepPending = true;
		printMenu();
		pthread_mutex_unlock(&printingMutex);
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
