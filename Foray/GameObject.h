#pragma once

#include "Physics/Collider.h"

class GameObject
{
public:
	virtual const void OnCollision(Collider* collider) = 0;
};