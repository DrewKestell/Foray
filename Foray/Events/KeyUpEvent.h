#pragma once

#include "KeyPressEvent.h"

class KeyUpEvent : public KeyPressEvent
{
public:
	KeyUpEvent(const WPARAM charCode)
		: KeyPressEvent(EventType::KeyUp, charCode)
	{
	}
};