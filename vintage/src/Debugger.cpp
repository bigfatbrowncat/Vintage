#include "Debugger.h"

Debugger::Debugger(FILE* debug_symbols, SDLScreen& screen) :
	activeWindow(dawCode),
	savedFlowLayer(0),
	screen(screen),
	state(dsStopped),
	topSpace(19), wStackBytes(8), wHeapBytes(10), wStackTopRow(0), wHeapTopRow(0),
	wSelectedLine(0)
{
	int pthred_res = pthread_mutex_init(&printingMutex, NULL);

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
			if (read_char != 0 && (read_char != '\n' || first_char == false) && read_char != '\r') 	// We remove first carriage return
			{
				lines += read_char;
			}
			if (read_char != '\n' && read_char != '\r') first_char = false;
		}
		while (read_char != 0);

		DebuggingSymbolsEntry newEntry;
		newEntry.memPos = mem_pos;
		newEntry.codeLine = lines;
		entries.push_back(newEntry);
	}
}

Debugger::~Debugger()
{
	pthread_mutex_destroy(&printingMutex);
}

int Debugger::findLine(int4 mem_pos) const
{
	for (unsigned int i = 0; i < entries.size() - 1; i++)
	{
		if (entries[i + 1].memPos > mem_pos &&
			entries[i].memPos <= mem_pos)
		{
			return i;
		}
	}
	return entries.size();
}

void Debugger::printMenu()
{
	screen.SelectForeColor(0, 0, 0);
	screen.SelectBackColor(192, 192, 192);
	printFixed(screen.getFrameBufferWidth() - 25, screen.getFrameBufferHeight() - 1, L" Hardware debugging tool ", 25);
	screen.SetCursorPosition(0, screen.getFrameBufferHeight() - 1);

	if (state == dsRunningPending || state == dsRunning)
	{
		screen.SelectBackColor(192, 192, 192);
	}
	else
	{
		screen.SelectBackColor(128, 128, 128);
	}
	screen.Write(L"1 Run    ");
	screen.SelectBackColor(0, 0, 0);
	screen.Write(L" ");

	if (state == dsStopped)
	{
		screen.SelectBackColor(192, 192, 192);
	}
	else
	{
		screen.SelectBackColor(128, 128, 128);
	}
	screen.Write(L"2 Pause  ");
	screen.SelectBackColor(0, 0, 0);
	screen.Write(L" ");

	if (state == dsStepOverPending || state == dsRunningOver)
	{
		screen.SelectBackColor(192, 192, 192);
	}
	else
	{
		screen.SelectBackColor(128, 128, 128);
	}
	screen.Write(L"3 Stp ovr");
	screen.SelectBackColor(0, 0, 0);
	screen.Write(L" ");

	if (state == dsStepIntoPending)
	{
		screen.SelectBackColor(192, 192, 192);
	}
	else
	{
		screen.SelectBackColor(128, 128, 128);
	}
	screen.Write(L"4 Stp in ");
	screen.SelectBackColor(0, 0, 0);
	screen.Write(L" ");

	if (state == dsStepOutPending || state == dsRunningOut)
	{
		screen.SelectBackColor(192, 192, 192);
		screen.SelectForeColor(0, 0, 0);
	}
	else if (flowLayers.size() > 0)
	{
		screen.SelectBackColor(128, 128, 128);
		screen.SelectForeColor(0, 0, 0);
	}
	else
	{
		screen.SelectBackColor(0, 0, 0);
		screen.SelectForeColor(128, 128, 128);
	}

	screen.Write(L"5 Stp out");
	screen.SelectBackColor(0, 0, 0);
	screen.Write(L" ");

	if (findBreakpointAt(entries[wSelectedLine].memPos) != breakpoints.end())
	{
		screen.SelectBackColor(192, 192, 192);
		screen.SelectForeColor(0, 0, 0);
	}
	else
	{
		screen.SelectBackColor(128, 128, 128);
		screen.SelectForeColor(0, 0, 0);
	}

	screen.Write(L"6 Breakpt");
	screen.SelectBackColor(0, 0, 0);
	screen.Write(L" ");

	screen.SelectForeColor(0, 0, 0);
	screen.SelectBackColor(128, 128, 128);
	screen.Write(L"7 Halt   ");

	screen.SelectBackColor(0, 0, 0);
	screen.Write(L" ");
	screen.SetCursorPosition(0, screen.getFrameBufferHeight() - 1);
}

void Debugger::printFixed(int x, int y, const wchar_t* str, int length)
{
	SymbolPlace* sp = screen.LockFrameBuffer();
	int pos = y * screen.getFrameBufferWidth() + x;

	bool eoln = false;
	for (int i = 0; i < length; i++)
	{
		if (!eoln && (str[i] == 0 || str[i] == L'\r' || str[i] == L'\n'))
		{
			eoln = true;
		}

		if (eoln && pos < screen.getFrameBufferWidth() * screen.getFrameBufferHeight())
		{
			sp[pos] = screen.createSymbolPlaceWithCurrentColors(0);
			pos++;
		}
		else
		{
			sp[pos] = screen.createSymbolPlaceWithCurrentColors(str[i]);
			pos++;
		}
	}

	screen.UnlockFrameBuffer();
}

void Debugger::updateUI()
{
	pthread_mutex_lock(&printingMutex);
	if ((screen.isActive() && state == dsRunning) || state == dsStopped)
	{
		// *** Printing code ***

		if (activeWindow == dawCode)
		{
			screen.SelectForeColor(0, 0, 0);
			screen.SelectBackColor(192, 192, 192);
		}
		else
		{
			screen.SelectForeColor(192, 192, 192);
			screen.SelectBackColor(96, 96, 96);
		}
		printFixed(0, topSpace - 1, L" Code ", screen.getFrameBufferWidth());

		int code_lines = screen.getFrameBufferHeight() - topSpace;
		int up = code_lines / 2;
		int down = code_lines - up - 2;

		int prev_addr = 0;
		for (int i = down; i >= -up; i--)
		{
			int index = wSelectedLine/*findLine(flow)*/ + i;

			int y = topSpace + up + i;

			if (i == 0)	// this line is selected now
			{
				screen.SelectForeColor(0, 0, 0);
				screen.SelectBackColor(192, 192, 192);
			}
			else if (index == findLine(flow)) // flow is here
			{
				screen.SelectForeColor(0, 0, 0);
				screen.SelectBackColor(128, 128, 255);
			}
			else if (findBreakpointAt(entries[index].memPos) != breakpoints.end()) // flow is here
			{
				screen.SelectForeColor(192, 0, 0);
				screen.SelectBackColor(0, 0, 0);
			}
			else // just a line
			{
				screen.SelectForeColor(128, 128, 128);
				screen.SelectBackColor(0, 0, 0);
			}

			wstringstream strs;
			if (index >= 0 && index < (int)entries.size() && entries[index].memPos != prev_addr)
			{
				strs << std::setw(8) << setfill(L'0') << std::hex << uppercase << entries[index].memPos << L" ";
				prev_addr = entries[index].memPos;
			}
			else
			{
				strs << L"         ";
			}
			printFixed(1, y, strs.str().c_str(), 9);

			if (i == 0)	// this line is selected now
			{
				screen.SelectForeColor(0, 0, 0);
				screen.SelectBackColor(192, 192, 192);
			}
			else if (index == findLine(flow)) // flow is here
			{
				screen.SelectForeColor(0, 0, 0);
				screen.SelectBackColor(128, 128, 255);
			}
			else if (findBreakpointAt(entries[index].memPos) != breakpoints.end()) // flow is here
			{
				screen.SelectForeColor(255, 0, 0);
				screen.SelectBackColor(0, 0, 0);
			}
			else // just a line
			{
				screen.SelectForeColor(192, 192, 192);
				screen.SelectBackColor(0, 0, 0);
			}

			wstringstream strs2;
			if (index >= 0 && index < (int)entries.size())
			{
				//printf("0x%X: %s\n", fl->mem_pos, fl->lines.c_str());
				strs2 << entries[index].codeLine << L"\n";
			}
			else
			{
				//printf("0x%X: <<No such address in debug symbols>>\n");
				wstringstream strs2;
				strs2 << L"<< No such address in debug symbols >>\n";
			}
			printFixed(10, y, strs2.str().c_str(), screen.getFrameBufferWidth() - 11);
		}

		printMenu();


		// ** Printing stack **

		int wStackWindowWidth = 9 + 3 * wStackBytes + wStackBytes + 2;

		if (activeWindow == dawStack)
		{
			screen.SelectForeColor(0, 0, 0);
			screen.SelectBackColor(192, 192, 192);
		}
		else
		{
			screen.SelectForeColor(192, 192, 192);
			screen.SelectBackColor(96, 96, 96);
		}
		printFixed(0, 0, L" Stack ", wStackWindowWidth);

		screen.SelectBackColor(0, 0, 0);

		for (int row = wStackTopRow; row < wStackTopRow + topSpace - 3; row++)
		{
			int stack_addr = row * wStackBytes;

			wstringstream strs;
			strs << std::setw(8) << setfill(L'0') << std::hex << uppercase << stack_addr << L" ";

			screen.SelectForeColor(128, 128, 128);
			printFixed(1, row - wStackTopRow + 1, strs.str().c_str(), 9);

			wstringstream strs2;
			wstringstream strs3;

			for (; stack_addr < (row + 1) * wStackBytes; stack_addr ++)
			{
				if (stack_addr < stackAllocatedSize)
				{
					unsigned char i1 = *((unsigned char*)(&stack[stack_addr]));

					//wstringstream strs;
					//strs << std::setw(2) << setfill(L'0') << std::hex << uppercase << i1 << " ";
					const wchar_t* hexchars = L"0123456789ABCDEF";
					wchar_t num[4];
					num[3] = 0;
					num[2] = L' ';
					num[1] = hexchars[i1 % 16];
					num[0] = hexchars[i1 / 16];

					wchar_t ch[2];
					if (i1 == 0)
					{
						ch[0] = ' ';
					}
					else
					{
						ch[0] = i1;
					}
					ch[1] = 0;

					screen.SelectForeColor(192, 192, 192);

					strs2 << num;
					strs3 << ch;
					//printFixed(9 + 1 + 3 * (stack_addr % wStackBytes), row - wStackTopRow + 1, num, 3);
					//printFixed(9 + 1 + wStackBytes * 3 + (stack_addr % wStackBytes), row - wStackTopRow + 1, ch, 1);

				}
				else
				{
					strs2 << L"   ";
					strs3 << L" ";

					//printFixed(9 + 1 + 3 * (stack_addr % wStackBytes), row - wStackTopRow + 1, L"   ", 3);
					//printFixed(9 + 1 + wStackBytes * 3 + (stack_addr % wStackBytes), row - wStackTopRow + 1, L" ", 1);
				}
			}

			strs2 << strs3.str();
			printFixed(9 + 1, row - wStackTopRow + 1, strs2.str().c_str(), wStackWindowWidth - 9);
		}

		// ** Printing heap **

		int wHeapWindowWidth = 2 + 9 + 3 * wHeapBytes + wHeapBytes;

		if (activeWindow == dawHeap)
		{
			screen.SelectForeColor(0, 0, 0);
			screen.SelectBackColor(192, 192, 192);
		}
		else
		{
			screen.SelectForeColor(192, 192, 192);
			screen.SelectBackColor(96, 96, 96);
		}
		printFixed(wStackWindowWidth + 2, 0, L" Heap ", wHeapWindowWidth);

		screen.SelectBackColor(0, 0, 0);

		for (int row = wHeapTopRow; row < wHeapTopRow + topSpace - 3; row++)
		{
			int heap_addr = row * wHeapBytes;

			wstringstream strs;
			strs << L" " << std::setw(8) << setfill(L'0') << std::hex << uppercase << heap_addr << L" ";

			screen.SelectForeColor(128, 128, 128);
			printFixed(wStackWindowWidth + 2, row - wHeapTopRow + 1, strs.str().c_str(), 9);

			wstringstream strs2;
			wstringstream strs3;

			for (; heap_addr < (row + 1) * wHeapBytes; heap_addr ++)
			{
				if (heap_addr < heapSize)
				{
					unsigned char i1 = *((unsigned char*)(&heap[heap_addr]));
					//wstringstream strs;
					//strs << std::setw(2) << setfill(L'0') << std::hex << uppercase << i1 << " ";
					const wchar_t* hexchars = L"0123456789ABCDEF";
					wchar_t num[4];
					num[3] = 0;
					num[2] = L' ';
					num[1] = hexchars[(i1 / 0x1)  & 0xF];
					num[0] = hexchars[(i1 / 0x10) & 0xF] ;

					wchar_t ch[2];
					if (i1 == 0)
					{
						ch[0] = ' ';
					}
					else
					{
						ch[0] = i1;
					}
					ch[1] = 0;

					screen.SelectForeColor(192, 192, 192);

					strs2 << num;
					strs3 << ch;
					//printFixed(wStackWindowWidth + 3 + 9 + 3 * (heap_addr % wHeapBytes), row - wHeapTopRow + 1, num, 3);
					//printFixed(wStackWindowWidth + 3 + 9 + wHeapBytes * 3 + (heap_addr % wHeapBytes), row - wStackTopRow + 1, ch, 1);
				}
				else
				{
					strs2 << L"   ";
					strs3 << L" ";
					//printFixed(wStackWindowWidth + 3 + 9 + 3 * (heap_addr % wHeapBytes), row - wHeapTopRow + 1, L"   ", 3);
					//printFixed(wStackWindowWidth + 3 + 9 + wHeapBytes * 3 + (heap_addr % wHeapBytes), row - wStackTopRow + 1, L" ", 1);
				}
			}

			strs2 << strs3.str();
			printFixed(wStackWindowWidth + 3 + 9, row - wHeapTopRow + 1, strs2.str().c_str(), wHeapWindowWidth - 9);
		}

	}
	pthread_mutex_unlock(&printingMutex);
}

void Debugger::reportFlowStateChanged(FlowState flowState)
{
	//printf("LVL: %d\n", savedFlowLayer);
	switch (flowState)
	{
	case fsStepIn:
		flowLayers.push_back(fltNormal);
		break;
	case fsStepOut:
		if (flowLayers.back() == fltNormal)
			flowLayers.pop_back();
		else
			printf("Incorrect flow layer!");
		break;
	case fsStepInHandler:
		flowLayers.push_back(fltHandler);
		break;
	case fsStepOutHandler:
		if (flowLayers.back() == fltHandler)
			flowLayers.pop_back();
		else
			printf("Incorrect flow layer!");
		break;
	case fsLinear:
		// Do nothing
		break;
	}
}

void Debugger::reportCPUState(int4 flow, int1* stack, int4 stackMaxSize, int4 stackAllocatedSize, int1* heap, int4 heapSize)
{
	this->flow = flow;

	this->stack = stack;
	this->stackAllocatedSize = stackAllocatedSize;

	this->stackMaxSize = stackMaxSize;

	this->heap = heap;
	this->heapSize = heapSize;

	updateUI();
}

const DebuggerOrder Debugger::askForOrder()
{
	//pthread_mutex_lock(&printingMutex);
	DebuggerOrder res;

	switch (state)
	{
	case dsStopped:
		res = doWait;
		break;
	case dsHaltPending:
		res = doHalt;
		break;
	case dsRunningPending:
		state = dsRunning;
		res = doGo;
		break;
	case dsStepIntoPending:
		savedFlowLayer = flowLayers.size();
		state = dsRunningInto;
		res = doGo;
		break;
	case dsStepOutPending:
		savedFlowLayer = flowLayers.size();
		state = dsRunningOut;
		res = doGo;
		break;
	case dsStepOverPending:
		savedFlowLayer = flowLayers.size();
		state = dsRunningOver;
		res = doGo;
		break;
	case dsRunning:
		// Checking for a breakpoint
		if (findBreakpointAt(flow) != breakpoints.end())
		{
			state = dsStopped;
			res = doWait;
		}
		else
		{
			// Continue running
			res = doGo;
		}
		break;
	case dsRunningInto:
		if (flowLayers.size() < 2 || flowLayers[flowLayers.size() - 2] == flowLayers[flowLayers.size() - 1] || findBreakpointAt(flow) != breakpoints.end())
		{
			state = dsStopped;
			res = doWait;
		}
		else
		{
			res = doGo;
		}
		break;
	case dsRunningOut:
		if (savedFlowLayer > flowLayers.size() || findBreakpointAt(flow) != breakpoints.end())
		{
			state = dsStopped;
			res = doWait;
		}
		else
		{
			res = doGo;
		}
		break;
	case dsRunningOver:
//		printf("OVER: size=%d, saved=%d, ", flowLayers.size(), savedFlowLayer);
		if (flowLayers.size() == 0 || savedFlowLayer == flowLayers.size() || findBreakpointAt(flow) != breakpoints.end())
		{
			state = dsStopped;
			res = doWait;
		}
		else
		{
			res = doGo;
		}
//		printf("state=%d, res=%d\n", state, res);
		fflush(stdout);
		break;
	}

	printMenu();
	//pthread_mutex_unlock(&printingMutex);
	return res;
}

void Debugger::handleControlKey(ControlKey ck)
{
	pthread_mutex_lock(&printingMutex);
	int heapLines = heapSize / wHeapBytes;
	int stackLines = stackMaxSize / wStackBytes;
	int codeWindowHeight = 20;

	if (ck == ckTab)
	{
		activeWindow = (DebuggerActiveWindow)((activeWindow + 1) % dawSize);
	}

	else if (ck == ckUp && activeWindow == dawCode)
	{
		wSelectedLine --;
	}
	else if (ck == ckDown && activeWindow == dawCode)
	{
		wSelectedLine ++;
	}
	else if (ck == ckPageUp && activeWindow == dawCode)
	{
		wSelectedLine -= codeWindowHeight / 2;
	}
	else if (ck == ckPageDown && activeWindow == dawCode)
	{
		wSelectedLine += codeWindowHeight / 2;
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

	if (wSelectedLine < 0) wSelectedLine = 0;
	if (wSelectedLine > lastLine()) wSelectedLine = lastLine();

	if (wHeapTopRow < 0) wHeapTopRow = 0;
	if (wHeapTopRow > heapLines) wHeapTopRow = heapLines;

	if (wStackTopRow < 0) wStackTopRow = 0;
	if (wStackTopRow > stackLines) wStackTopRow = stackLines;

	pthread_mutex_unlock(&printingMutex);

	updateUI();
}

vector<Breakpoint>::iterator Debugger::findBreakpointAt(int4 memPos)
{
	for (vector<Breakpoint>::iterator iter = breakpoints.begin(); iter != breakpoints.end(); iter++)
	{
		if ((*iter).memPos == memPos) return iter;
	}
	return breakpoints.end();
}

void Debugger::toggleBreakpointAt(int4 memPos)
{
	vector<Breakpoint>::iterator bp = findBreakpointAt(memPos);
	if (bp != breakpoints.end())
	{
		breakpoints.erase(bp);
	}
	else
	{
		breakpoints.push_back(Breakpoint(memPos));
	}
}

void Debugger::run()
{
	if (state != dsRunning && state != dsRunningOut && state != dsRunningOver)
	{
		pthread_mutex_lock(&printingMutex);
		state = dsRunningPending;
		printMenu();
		pthread_mutex_unlock(&printingMutex);
	}
}
void Debugger::stop()
{
	if (state == dsRunning)
	{
		pthread_mutex_lock(&printingMutex);
		state = dsStopped;
		printMenu();
		pthread_mutex_unlock(&printingMutex);
	}
}
void Debugger::stepOver()
{
	if (state != dsRunningOut && state != dsRunningOver)
	{
		pthread_mutex_lock(&printingMutex);
		state = dsStepOverPending;
		printMenu();
		pthread_mutex_unlock(&printingMutex);
	}
}
void Debugger::stepInto()
{
	if (state != dsRunningOut && state != dsRunningOver)
	{
		pthread_mutex_lock(&printingMutex);
		state = dsStepIntoPending;
		printMenu();
		pthread_mutex_unlock(&printingMutex);
	}
}
void Debugger::stepOut()
{
	if (state != dsRunningOut && state != dsRunningOver && flowLayers.size() > 0)
	{
		pthread_mutex_lock(&printingMutex);
		state = dsStepOutPending;
		printMenu();
		pthread_mutex_unlock(&printingMutex);
	}
}
void Debugger::halt()
{
	pthread_mutex_lock(&printingMutex);
	state = dsHaltPending;
	printMenu();
	pthread_mutex_unlock(&printingMutex);
}
void Debugger::toggleBreakpoint()
{
	toggleBreakpointAt(entries[wSelectedLine].memPos);
	updateUI();
}
