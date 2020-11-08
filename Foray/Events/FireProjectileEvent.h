#pragma once

#include "Event.h"

class FireProjectileEvent : public Event
{
public:
	FireProjectileEvent(const unsigned short ownerId, const XMFLOAT2 position, const XMFLOAT2 velocity)
		: Event(EventType::FireProjectile),
		  ownerId{ ownerId },
		  position{ position },
		  velocity{ velocity }
	{
	}

	const unsigned short ownerId;

	const XMFLOAT2 position;

	const XMFLOAT2 velocity;
};