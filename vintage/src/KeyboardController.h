class KeyboardController;

#ifndef KEYBOARDCONTROLLER_H_
#define KEYBOARDCONTROLLER_H_

#include "KeyModifiers.h"

class KeyboardController
{
public:
	virtual void ChangeKeyState(bool key_down, KeyModifiers modifiers, int4 key_code) = 0;
};

#endif
