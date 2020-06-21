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


PhysicsEngine::~PhysicsEngine()
{
	eventHandler.Unsubscribe(*this);
}