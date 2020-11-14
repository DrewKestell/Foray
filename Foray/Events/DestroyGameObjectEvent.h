#pragma once

#include "Event.h"

class DestroyGameObjectEvent : public Event
{
public:
	DestroyGameObjectEvent(const unsigned short gameObjectId)
		: Event(EventType::DestroyGameObject),
		GameObjectId{ gameObjectId }
	{
	}

	const unsigned int GameObjectId;
};