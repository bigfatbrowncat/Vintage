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

class Debugger
{
private:
	vector<DebugEntry> entries;
	SDLScreen& screen;
	volatile bool running;
	volatile bool haltPending;
	volatile bool stepPending;
	pthread_mutex_t printing_mutex;
protected:
	void printMenu();
	void writeFixed(const wchar_t* str, int length);
public:
	const vector<DebugEntry>& getEntries() const { return entries; }
	Debugger(FILE* debug_symbols, SDLScreen& screen);
	virtual ~Debugger();
	int findLine(int4 mem_pos) const;
	void stepDone(int4 flow);
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
