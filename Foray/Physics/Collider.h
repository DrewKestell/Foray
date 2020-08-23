#pragma once

#include <d2d1_3.h>
#include "../Events/Observer.h"
#include "../Events/EventHandler.h"
#include "../GameObject.h"

class Collider : public Observer
{
private:
	// constructor params
	EventHandler& eventHandler;

	// locals
	const unsigned int id;
	D2D1_RECT_F rect;
	GameObject* gameObject;

public:
	Collider(EventHandler& eventHandler, const unsigned int id, const D2D1_RECT_F rect, GameObject* gameObject);
	virtual const void HandleEvent(const Event* const event);
	const unsigned int GetId() const;
	const D2D1_RECT_F GetRect() const;
	const void SetRect(const D2D1_RECT_F rect);
	GameObject* GetGameObject();
	~Collider();
};