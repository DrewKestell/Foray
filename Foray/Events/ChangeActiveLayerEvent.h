#pragma once

#include "Event.h"
#include "../UI/Layer.h"

class ChangeActiveLayerEvent : public Event
{
public:
	ChangeActiveLayerEvent(const Layer layer)
		: Event(EventType::ChangeActiveLayer),
		Layer{ layer }
	{
	}
	const Layer Layer;
};