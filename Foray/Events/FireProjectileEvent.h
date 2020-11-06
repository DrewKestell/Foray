#pragma once

#include <DirectXMath.h>
#include "Event.h"

using namespace DirectX;

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