#include "../stdafx.h"
#include "PhysicsComponent.h"

PhysicsComponent::PhysicsComponent(
	GameObject& gameObject,
	const XMFLOAT2 position,
	const XMFLOAT2 velocity)
	: gameObject{ gameObject },
	  Position{ position },
	  Velocity{ velocity }
{
}

GameObject& PhysicsComponent::GetGameObject()
{
	return gameObject;
}