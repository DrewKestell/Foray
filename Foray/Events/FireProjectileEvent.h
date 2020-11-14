#pragma once

#include "Event.h"

class FireProjectileEvent : public Event
{
public:
	FireProjectileEvent(const unsigned int ownerId, const XMFLOAT2 position, const XMFLOAT2 velocity)
		: Event(EventType::FireProjectile),
		  OwnerId{ ownerId },
		  Position{ position },
		  Velocity{ velocity }
	{
	}

	const unsigned int OwnerId;

	const XMFLOAT2 Position;

	const XMFLOAT2 Velocity;
};