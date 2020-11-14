#pragma once

#include "Event.h"

class MouseEvent : public Event
{
public:
	MouseEvent(const EventType eventType, const float mousePosX, const float mousePosY)
		: Event(eventType),
		MousePosX{ mousePosX },
		MousePosY{ mousePosY }
	{
	}

	const float MousePosX;
	const float MousePosY;
};