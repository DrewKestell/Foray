#pragma once

#include "ColliderType.h"
#include "../Events/Observer.h"

// forward declare GameObject to avoid circular reference between GameObject and Collider
class GameObject;

class Collider : public Observer
{
private:
	// constructor params
	GameObject& gameObject;
	const bool registerCollider;
	
public:
	ColliderType Type;
	const float Width;
	const float Height;
	XMFLOAT2 Position;

	Collider(
		GameObject& gameObject,
		ColliderType type,
		const float width,
		const float height,
		const XMFLOAT2 position,
		const bool registerCollider);
	GameObject& GetGameObject();
	const D2D1_RECT_F GetRect() const;
	virtual const bool HandleEvent(const Event* const event);
	~Collider();
};