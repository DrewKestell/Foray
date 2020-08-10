#pragma once

#include "../Events/Observer.h"
#include "../Events/EventHandler.h"
#include "Collider.h"

class PhysicsEngine : public Observer
{
private:
	// constructor params
	EventHandler& eventHandler;

	//locals
	std::vector<Collider*> colliders;

public:
	PhysicsEngine(EventHandler& eventHandler);
	virtual const void HandleEvent(const Event* const event);
	void RegisterCollider(Collider* collider);
	void UnregisterCollider(Collider* collider);
	void Update();
	~PhysicsEngine();
};