#include "Debugger.h"

Debugger::Debugger(FILE* debug_symbols, SDLScreen& screen) :
	screen(screen),
	running(false),
	haltPending(false),
	stepOverPending(false),
	stepIntoPending(false),
	stepOutPending(false),
	runningOut(false),
	runningOver(false),
	topSpace(19), wStackBytes(8), wHeapBytes(10), wStackTopRow(0), wHeapTopRow(0),
	flowLevel(0)
{
	pthread_mutex_init(&printingMutex, NULL);

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

Debugger::~Debugger()
{
	pthread_mutex_destroy(&printingMutex);
}

int Debugger::findLine(int4 mem_pos) const
{
	for (unsigned int i = 0; i < entries.size() - 1; i++)
	{
		if (entries[i + 1].mem_pos > mem_pos &&
			entries[i].mem_pos <= mem_pos)
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
	if (this->running)
	{
		screen.SelectBackColor(192, 192, 192);
	}
	else
	{
		screen.SelectBackColor(128, 128, 128);
	}
	screen.Write(L"1 Run      ");
	screen.SelectBackColor(0, 0, 0);
	screen.Write(L" ");

	if (!running &&
	    !stepOverPending &&
	    !stepIntoPending &&
	    !stepOutPending &&
	    !runningOut &&
	    !runningOver)
	{
		screen.SelectBackColor(192, 192, 192);
	}
	else
	{
		screen.SelectBackColor(128, 128, 128);
	}
	screen.Write(L"2 Pause    ");
	screen.SelectBackColor(0, 0, 0);
	screen.Write(L" ");

	if (stepOverPending || runningOver)
	{
		screen.SelectBackColor(192, 192, 192);
	}
	else
	{
		screen.SelectBackColor(128, 128, 128);
	}
	screen.Write(L"3 Step over");
	screen.SelectBackColor(0, 0, 0);
	screen.Write(L" ");

	if (stepIntoPending)
	{
		screen.SelectBackColor(192, 192, 192);
	}
	else
	{
		screen.SelectBackColor(128, 128, 128);
	}
	screen.Write(L"4 Step into");
	screen.SelectBackColor(0, 0, 0);
	screen.Write(L" ");

	if (stepOutPending || runningOut)
	{
		screen.SelectBackColor(192, 192, 192);
		screen.SelectForeColor(0, 0, 0);
	}
	else if (flowLevel > 0)
	{
		screen.SelectBackColor(128, 128, 128);
		screen.SelectForeColor(0, 0, 0);
	}
	else
	{
		screen.SelectBackColor(0, 0, 0);
		screen.SelectForeColor(128, 128, 128);
	}

	screen.Write(L"5 Step out ");
	screen.SelectBackColor(0, 0, 0);
	screen.Write(L" ");

	screen.SelectForeColor(0, 0, 0);
	screen.SelectBackColor(128, 128, 128);
	screen.Write(L"6 Halt CPU ");
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
	if (screen.isActive() || !running)
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
			int index = findLine(flow) + i;

			int y = topSpace + up + i;

			if (i != 0)
			{
				screen.SelectForeColor(128, 128, 128);
				screen.SelectBackColor(0, 0, 0);
			}
			else
			{
				screen.SelectForeColor(0, 0, 0);
				screen.SelectBackColor(192, 192, 192);
			}

			wstringstream strs;
			if (index >= 0 && index < (int)entries.size() && entries[index].mem_pos != prev_addr)
			{
				strs << std::setw(8) << setfill(L'0') << std::hex << uppercase << entries[index].mem_pos << L" ";
				prev_addr = entries[index].mem_pos;
			}
			else
			{
				strs << L"         ";
			}
			printFixed(1, y, strs.str().c_str(), 9);

			if (i != 0)
			{
				screen.SelectForeColor(192, 192, 192);
				screen.SelectBackColor(0, 0, 0);
			}
			else
			{
				screen.SelectBackColor(0, 0, 0);
				screen.SelectBackColor(192, 192, 192);
			}

			wstringstream strs2;
			if (index >= 0 && index < (int)entries.size())
			{
				//printf("0x%X: %s\n", fl->mem_pos, fl->lines.c_str());
				strs2 << entries[index].lines << L"\n";
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

void Debugger::reportFlowStateEvent(FlowState flowState)
{
	if (flowState == fsStepIn)
		flowLevel ++;
	if (flowState == fsStepOut)
		flowLevel --;
}

void Debugger::flowChanged(int4 flow, int1* stack, int4 stackMaxSize, int4 stackAllocatedSize, int1* heap, int4 heapSize)
{

	this->flow = flow;

	this->stack = stack;
	this->stackAllocatedSize = stackAllocatedSize;

	this->stackMaxSize = stackMaxSize;

	this->heap = heap;
	this->heapSize = heapSize;

	updateUI();
}
