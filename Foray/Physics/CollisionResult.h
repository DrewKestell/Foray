#pragma once

class Collider;

const unsigned char COLLISION_DIRECTION_NONE   = 0x00;
const unsigned char COLLISION_DIRECTION_TOP    = 0x01;
const unsigned char COLLISION_DIRECTION_RIGHT  = 0x02;
const unsigned char COLLISION_DIRECTION_BOTTOM = 0x04;
const unsigned char COLLISION_DIRECTION_LEFT   = 0x08;

class CollisionResult
{
	unsigned char collisionDirection;
	Collider* collider;
public:
	CollisionResult(unsigned char collisionDirection, Collider* collider)
		: collisionDirection{ collisionDirection },
		  collider{ collider }
	{};
	const unsigned char GetCollisionDirection() const { return collisionDirection; }
	Collider* GetCollider() { return collider; }
};