#include "../stdafx.h"
#include "Collider.h"

Collider::Collider(EventHandler& eventHandler, const unsigned int id, const D2D1_RECT_F rect, GameObject* gameObject)
	: eventHandler{ eventHandler },
	  id{ id },
	  rect{ rect },
	  gameObject{ gameObject }
{
	eventHandler.Subscribe(*this);
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

Collider::~Collider()
{
	eventHandler.Unsubscribe(*this);
}