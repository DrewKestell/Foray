#include "stdafx.h"
#include "Collider.h"

Collider::Collider(EventHandler& eventHandler, const D2D1_RECT_F rect)
	: eventHandler{ eventHandler },
	  rect{ rect }
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