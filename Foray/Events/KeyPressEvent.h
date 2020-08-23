#pragma once

#include "Event.h"

class KeyPressEvent : public Event
{
public:
	KeyPressEvent(const EventType eventType, const WPARAM charCode)
		: Event(eventType),
		  charCode{ charCode }
	{
	}
	const WPARAM charCode;
};