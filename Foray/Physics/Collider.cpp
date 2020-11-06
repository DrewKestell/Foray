#include "../stdafx.h"
#include "Collider.h"
#include "../Events/EventHandler.h"

extern std::unique_ptr<EventHandler> g_eventHandler;

Collider::Collider(const unsigned int id, const D2D1_RECT_F rect, GameObject* gameObject, ColliderType colliderType)
	: id{ id },
	  rect{ rect },
	  gameObject{ gameObject },
	  colliderType{ colliderType }
{
	g_eventHandler->Subscribe(*this);
}


const void Collider::HandleEvent(const Event* const event)
{
	const auto type = event->type;
	switch (type)
	{
	
	}
}

const unsigned int Collider::GetId() const
{
	return id;
}

const D2D1_RECT_F Collider::GetRect() const
{
	return rect;
}

const void Collider::SetRect(const D2D1_RECT_F rect)
{
	this->rect = rect;
}

GameObject* Collider::GetGameObject()
{
	return gameObject;
}

ColliderType Collider::GetColliderType()
{
	return colliderType;
}

Collider::~Collider()
{
	g_eventHandler->Unsubscribe(*this);
}