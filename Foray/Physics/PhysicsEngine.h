#pragma once

#include "../Events/Observer.h"
#include "../Events/EventHandler.h"

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
	~PhysicsEngine();
};