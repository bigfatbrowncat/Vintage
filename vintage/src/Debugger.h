class Debugger;
class DebuggerKeyboardController;

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

enum DebuggerOrder { Wait, Go, Halt };

class Screen;

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

enum DebuggerUIState
{
	Idle,
	AddWatchEditAddress,
	AddWatchEditLength
};

class Debugger
{
private:
	int top_space;
	DebuggerUIState state;
	vector<DebugEntry> entries;
	vector<WatchEntry> watches;
	SDLScreen& screen;
	volatile bool running;
	volatile bool haltPending;
	volatile bool stepPending;
	pthread_mutex_t printing_mutex;

	addr watch_add_address;
	int1 watch_add_bytes;

	int4 flow;

	int1* stack;
	int4 stackSize;

	int1* heap;
	int4 heapSize;

	int radix;

protected:
	void printMenu();
	void printFixed(int x, int y, const wchar_t* str, int length);
public:
	void updateWatchUI();
	void updateUI();

	bool isEditingWatchAddress()
	{
		return (state == AddWatchEditAddress);
	}
	bool isEditingWatchLength()
	{
		return (state == AddWatchEditLength);
	}

	void addWatch()
	{
		if (state == Idle)
		{
			watch_add_address = 0;
			watch_add_bytes = 8;
			selectWatchAddressEditor();
		}
	}
	void selectWatchAddressEditor()
	{
		state = AddWatchEditAddress;
		updateUI();
	}
	void inputDigit(char d)
	{
		if (state == AddWatchEditAddress)
		{
			if (radix == 16)
			{
				if (watch_add_address < 0x10000000)
				{
					watch_add_address = watch_add_address * 0x10 + d;
					updateUI();
				}
			}
			else if (radix == 10)
			{
				if (d < 10 && watch_add_address * 10 + d > watch_add_address)
				{
					watch_add_address = watch_add_address * 10 + d;
					updateUI();
				}
			}
		}
		else if (state == AddWatchEditLength)
		{
			watch_add_bytes = d;
			updateUI();
		}
	}
	void removeWatchAddressDigit()
	{
		if (radix == 16)
		{
			watch_add_address /= 0x10;
		}
		else if (radix == 10)
		{
			watch_add_address /= 10;
		}

		updateUI();
	}
	void selectWatchLengthEditor()
	{
		state = AddWatchEditLength;
		updateUI();
	}
	void completeAddWatch()
	{
		if (state == AddWatchEditAddress || state == AddWatchEditLength)
		{
			state = Idle;

			WatchEntry we;
			we.stack_pos = watch_add_address;
			we.length = watch_add_bytes;

			watches.push_back(we);
			updateUI();
		}
	}
	void changeSelection()
	{
		if (state == AddWatchEditAddress)
			selectWatchLengthEditor();
		else if (state == AddWatchEditLength)
			selectWatchAddressEditor();
	}

	void switchRadix()
	{
		if (radix == 16)
			radix = 10;
		else if (radix == 10)
			radix = 16;

		updateUI();
	}

	const vector<DebugEntry>& getEntries() const { return entries; }
	Debugger(FILE* debug_symbols, SDLScreen& screen);
	virtual ~Debugger();
	int findLine(int4 mem_pos) const;
	void stepDone(int4 flow, int1* stack, int4 stackSize, int1* heap, int4 heapSize);
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
		pthread_mutex_lock(&printing_mutex);
		running = true;
		printMenu();
		pthread_mutex_unlock(&printing_mutex);
	}
	void stop()
	{
		pthread_mutex_lock(&printing_mutex);
		running = false;
		printMenu();
		pthread_mutex_unlock(&printing_mutex);
	}
	void step()
	{
		pthread_mutex_lock(&printing_mutex);
		running = false;
		stepPending = true;
		printMenu();
		pthread_mutex_unlock(&printing_mutex);
	}
	void halt()
	{
		pthread_mutex_lock(&printing_mutex);
		haltPending = true;
		printMenu();
		pthread_mutex_unlock(&printing_mutex);
	}
};

#endif
