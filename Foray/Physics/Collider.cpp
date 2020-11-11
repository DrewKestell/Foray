#include "../stdafx.h"
#include "Collider.h"
#include "PhysicsEngine.h"
#include "../Events/EventHandler.h"

extern std::unique_ptr<EventHandler> g_eventHandler;
extern std::unique_ptr<PhysicsEngine> g_physicsEngine;

Collider::Collider(
	GameObject& gameObject,
	ColliderType type,
	const float width,
	const float height,
	const XMFLOAT2 position)
	: gameObject{ gameObject },
	  Type{ type},
	  Width{ width },
	  Height{ height },
	  Position{ position }
{
	g_eventHandler->Subscribe(*this);
	g_physicsEngine->RegisterCollider(this);
}

const void Collider::HandleEvent(const Event* const event)
{
	const auto type = event->type;
	switch (type)
	{
	
	}
}

const D2D1_RECT_F Collider::GetRect() const
{
	return D2D1_RECT_F
	{
		Position.x - (Width / 2),
		Position.y - (Height / 2),
		Position.x + (Width / 2),
		Position.y + (Height / 2)
	};
}

GameObject& Collider::GetGameObject()
{
	return gameObject;
}

Collider::~Collider()
{
	g_eventHandler->Unsubscribe(*this);
	g_physicsEngine->UnregisterCollider(this);
}