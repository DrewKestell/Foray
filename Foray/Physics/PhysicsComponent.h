#pragma once

#include "../GameObject.h"

class PhysicsComponent
{
	GameObject& gameObject;

public:
	XMFLOAT2 Position;
	XMFLOAT2 Velocity;

	PhysicsComponent(
		GameObject& gameObject,
		const XMFLOAT2 position,
		const XMFLOAT2 velocity);
	GameObject& GetGameObject();
};