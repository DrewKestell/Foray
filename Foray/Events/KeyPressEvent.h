#pragma once

#include "Event.h"

class KeyPressEvent : public Event
{
public:
	KeyPressEvent(const EventType eventType, const WPARAM charCode)
		: Event(eventType),
		  CharCode{ charCode }
	{
	}
	const WPARAM CharCode;
};