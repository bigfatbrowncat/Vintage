#ifndef DEBUGGERKEYBOARDCONTROLLER_H_
#define DEBUGGERKEYBOARDCONTROLLER_H_

#include "KeyModifiers.h"
#include "Debugger.h"
#include "KeyboardController.h"

class DebuggerKeyboardController : public KeyboardController
{
private:
	Debugger& debugger;
public:
	DebuggerKeyboardController(Debugger& debugger) : debugger(debugger) {}
	virtual ~DebuggerKeyboardController() {}
	void ChangeKeyState(bool key_down, KeyModifiers modifiers, int4 key_code);
};

#endif
