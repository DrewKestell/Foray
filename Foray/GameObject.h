#pragma once

#include "Physics/CollisionResult.h"

// forward declare Collider to avoid circular reference between Collider and GameObject
class Collider;

class GameObject
{
public:
	virtual const void OnCollision(CollisionResult collisionResult) = 0;
};