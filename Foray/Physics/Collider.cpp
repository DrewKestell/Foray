#include "../stdafx.h"
#include "Collider.h"

Collider::Collider(EventHandler& eventHandler, const D2D1_RECT_F rect, GameObject* gameObject)
	: eventHandler{ eventHandler },
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


Collider::~Collider()
{
	eventHandler.Unsubscribe(*this);
}