#include "../stdafx.h"
#include "PhysicsEngine.h"

PhysicsEngine::PhysicsEngine(EventHandler& eventHandler)
	: eventHandler{ eventHandler }
{
	eventHandler.Subscribe(*this);
}


const void PhysicsEngine::HandleEvent(const Event* const event)
{
	const auto type = event->type;
	switch (type)
	{

	}
}

void PhysicsEngine::RegisterCollider(Collider* collider)
{
	colliders.push_back(collider);
}

void PhysicsEngine::UnregisterCollider(Collider* collider)
{
	colliders.erase(std::remove(colliders.begin(), colliders.end(), collider), colliders.end());
}

void PhysicsEngine::Update()
{
	// collision detection
}

PhysicsEngine::~PhysicsEngine()
{
	eventHandler.Unsubscribe(*this);
}