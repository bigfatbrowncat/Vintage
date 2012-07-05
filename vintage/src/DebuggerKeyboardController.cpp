#include "DebuggerKeyboardController.h"

void DebuggerKeyboardController::ChangeKeyState(bool key_down, KeyModifiers modifiers, int4 key_code)
{
	if (key_down)
	{
		switch (key_code)
		{
		case SDLK_F1:
			debugger.run();
			break;
		case SDLK_F2:
			debugger.stop();
			break;
		case SDLK_F3:
			debugger.stepOver();
			break;
		case SDLK_F4:
			debugger.stepInto();
			break;
		case SDLK_F5:
			debugger.stepOut();
			break;
		case SDLK_F6:
			debugger.halt();
			break;

		case SDLK_TAB:
			debugger.handleControlKey(ckTab);
			break;
		case SDLK_LEFT:
			debugger.handleControlKey(ckLeft);
			break;
		case SDLK_UP:
			debugger.handleControlKey(ckUp);
			break;
		case SDLK_RIGHT:
			debugger.handleControlKey(ckRight);
			break;
		case SDLK_DOWN:
			debugger.handleControlKey(ckDown);
			break;
		case SDLK_PAGEUP:
			debugger.handleControlKey(ckPageUp);
			break;
		case SDLK_PAGEDOWN:
			debugger.handleControlKey(ckPageDown);
			break;

		}

	}
}
