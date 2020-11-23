#include "../stdafx.h"
#include "PhysicsEngine.h"
#include "../Events/EventHandler.h"
#include "../GameObject.h"

extern std::unique_ptr<EventHandler> g_eventHandler;

PhysicsEngine::PhysicsEngine()
{
	g_eventHandler->Subscribe(*this);
}

const unsigned char PhysicsEngine::CheckRectangleOverlap(D2D1_RECT_F l, D2D1_RECT_F r) const
{
	auto collisionDirection = COLLISION_DIRECTION_NONE;

	// no collision
	if (l.left > r.right || l.right < r.left || l.bottom < r.top || l.top > r.bottom)
		return collisionDirection;

	const auto isAbove = l.top < r.top && l.bottom < r.bottom;
	const auto isBelow = l.top > r.top && l.bottom > r.bottom;
	const auto isRight = l.left > r.left && l.right > r.right;
	const auto isLeft  = l.left < r.left && l.right < r.right;

	if (isAbove && l.bottom > r.top && ((l.right > r.left && isLeft) || (l.left < r.right && isRight) || (l.left >= r.left && l.right <= r.right) || (l.left <= r.left && l.right >= r.right)))
		collisionDirection |= COLLISION_DIRECTION_BOTTOM;

	if (isBelow && l.top < r.bottom && ((l.right > r.left && isLeft) || (l.left < r.right && isRight) || (l.left >= r.left && l.right <= r.right) || (l.left <= r.left && l.right >= r.right)))
		collisionDirection |= COLLISION_DIRECTION_TOP;

	if (isRight && l.left < r.right && ((l.bottom > r.top && isAbove) || (l.top < r.bottom && isBelow) || (l.top > r.top && l.bottom < r.bottom) || (l.top < r.top && l.bottom > r.bottom)))
 	 	collisionDirection |= COLLISION_DIRECTION_LEFT;

	if (isLeft && l.right > r.left && ((l.bottom > r.top && isAbove) || (l.top < r.bottom && isBelow) || (l.top > r.top && l.bottom < r.bottom) || (l.top < r.top && l.bottom > r.bottom)))
		collisionDirection |= COLLISION_DIRECTION_RIGHT;

	return collisionDirection;
}

const bool PhysicsEngine::HandleEvent(const Event* const event)
{
	const auto type = event->Type;
	switch (type)
	{
		
	}

	return false;
}

void PhysicsEngine::RegisterCollider(Collider* collider)
{
	colliders.push_back(collider);
}

void PhysicsEngine::UnregisterCollider(Collider* collider)
{
	colliders.erase(std::remove(colliders.begin(), colliders.end(), collider), colliders.end());
}

void PhysicsEngine::Update()
{
	/*for (auto i = 0; i < colliders.size(); i++)
	{
		const auto c1 = colliders.at(i);
		auto collisionResult = CheckCollision(c1);
		if (collisionResult.GetCollider())
		{
			c1->GetGameObject()->OnCollision(collisionResult);
		}
	}*/
}

// TODO: return array of collisions?
CollisionResult PhysicsEngine::CheckCollision(Collider* collider) const
{
	for (auto i = 0; i < colliders.size(); i++)
	{
		auto otherCollider = colliders.at(i);
		if (collider->GetGameObject().GameObjectId != otherCollider->GetGameObject().GameObjectId)
		{
			auto collisionDirection = CheckRectangleOverlap(collider->GetRect(), otherCollider->GetRect());
			if (collisionDirection)
				return CollisionResult{ collisionDirection, otherCollider };
		}
	}
	return CollisionResult{ COLLISION_DIRECTION_NONE, nullptr };
}

const CollisionResult PhysicsEngine::CheckCollisionForPosition(
	GameObject& gameObject,
	const float width,
	const float height,
	const XMFLOAT2 proposedPos) const
{
	const auto proposedCollider = std::make_unique<Collider>(gameObject, ColliderType::Player, width, height, proposedPos, false);

	return CheckCollision(proposedCollider.get());
}

void PhysicsEngine::DrawColliders(DeviceResources* deviceResources, ID2D1SolidColorBrush* brush)
{
	auto d2dFactory = deviceResources->GetD2DFactory();
	auto d2dContext = deviceResources->GetD2DDeviceContext();
	ComPtr<ID2D1RectangleGeometry> geometry;

	for (auto i = 0; i < colliders.size(); i++)
	{
		auto collider = colliders.at(i);
		d2dFactory->CreateRectangleGeometry(collider->GetRect(), geometry.ReleaseAndGetAddressOf());
		d2dContext->DrawGeometry(geometry.Get(), brush);
	}
}

PhysicsEngine::~PhysicsEngine()
{
	g_eventHandler->Unsubscribe(*this);
}