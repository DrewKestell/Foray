#pragma once

#include "EventType.h"

class Event
{
public:
	Event(const EventType type)
		: Type{ type }
	{
	}
	const EventType Type;
	virtual ~Event() {}
};