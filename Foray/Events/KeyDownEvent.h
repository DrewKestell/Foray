#pragma once

#include "KeyPressEvent.h"

class KeyDownEvent : public KeyPressEvent
{
public:
	KeyDownEvent(const WPARAM charCode)
		: KeyPressEvent(EventType::KeyDown, charCode)
	{
	}
};