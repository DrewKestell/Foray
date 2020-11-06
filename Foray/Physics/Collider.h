#pragma once

#include <d2d1_3.h>
#include "ColliderType.h"
#include "../Events/Observer.h"
#include "../GameObject.h"

class Collider : public Observer
{
private:
	// constructor params
	const unsigned int id;
	D2D1_RECT_F rect;
	GameObject* gameObject;
	ColliderType colliderType;

public:
	Collider(const unsigned int id, const D2D1_RECT_F rect, GameObject* gameObject, ColliderType colliderType);
	virtual const void HandleEvent(const Event* const event);
	const unsigned int GetId() const;
	const D2D1_RECT_F GetRect() const;
	const void SetRect(const D2D1_RECT_F rect);
	GameObject* GetGameObject();
	ColliderType GetColliderType();
	~Collider();
};