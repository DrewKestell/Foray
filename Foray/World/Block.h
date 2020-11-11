#pragma once

#include "../Physics/Collider.h"
#include "../Physics/CollisionResult.h"
#include "../Events/Observer.h"

class Block : public Observer
{
private:
	// constructor params
	D2D1_ROUNDED_RECT rect;

	// locals
	GameObject* gameObject; // hack - fixme
	Collider* collider;
	ID2D1DeviceContext1* d2dContext{ nullptr };
	ID2D1SolidColorBrush* fillBrush{ nullptr };
	ComPtr<ID2D1RoundedRectangleGeometry> geometry;

public:
	Block(const D2D1_ROUNDED_RECT rect, GameObject* gameObject, Collider* collider);
	void Initialize(ID2D1DeviceContext1* d2dContext, ID2D1Factory2* d2dFactory, ID2D1SolidColorBrush* fillBrush);
	void Draw();
	virtual const void HandleEvent(const Event* const event);
	virtual const void OnCollision(CollisionResult collisionResult);
	~Block();
};