#pragma once

#include "Physics/CollisionResult.h"

// forward declare Collider to avoid circular reference between Collider and GameObject
class Collider;

static unsigned int IdCounter;

class GameObject
{
public:
	const unsigned int GameObjectId{ IdCounter++ };
	virtual const void OnCollision(CollisionResult collisionResult) = 0;
};