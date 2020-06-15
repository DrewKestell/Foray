#pragma once

#include <d2d1_3.h>
#include "Events/Observer.h"
#include "Events/EventHandler.h"

class Collider : public Observer
{
private:
	// constructor params
	EventHandler& eventHandler;

	// locals
	D2D1_RECT_F rect;

public:
	Collider(EventHandler& eventHandler, const D2D1_RECT_F rect);
	virtual const void HandleEvent(const Event* const event);
	~Collider();
};