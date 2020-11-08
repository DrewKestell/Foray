#pragma once

#include <vector>
#include "../Events/Observer.h"
#include "Collider.h"
#include "CollisionResult.h"
#include "../Graphics/DeviceResources.h"

class PhysicsEngine : public Observer
{
private:
	//locals
	std::vector<Collider*> colliders;

	const unsigned char CheckRectangleOverlap(D2D1_RECT_F l, D2D1_RECT_F r) const;

public:
	PhysicsEngine();
	virtual const void HandleEvent(const Event* const event);
	void RegisterCollider(Collider* collider);
	void UnregisterCollider(Collider* collider);
	void Update();
	CollisionResult CheckCollision(Collider* collider) const;
	void DrawColliders(DeviceResources* deviceResources, ID2D1SolidColorBrush* brush);
	~PhysicsEngine();
};