#pragma once

#include "Physics/Collider.h"
#include "Physics/CollisionResult.h"
#include "UI/Layer.h"
#include "Events/Observer.h"
#include "GameObject.h"

class Player : public Observer
{
private:
	GameObject& gameObject;

	// locals
	float moveAnimationTimer{ 0.0f };
	float shootAnimationTimer{ 0.0f };
	unsigned char moveFrame{ 1 };
	unsigned char lastMoveFrame{ 1 };

	// states (probably want to move this to a playerController or something)
	float verticalVelocity{ 0.0f };
	bool jumpPressed{ false };
	bool jumpReleased{ true };
	bool canJump{ true };
	bool landed{ true };
	bool leftPressed{ false };
	bool rightPressed{ false };
	bool movingLeft{ false };
	bool movingRight{ false };
	bool canShoot{ true };
	bool shootPressed{ false };
	bool shooting{ false };
	
public:
	Player(GameObject& gameObject);
	void Update();
	virtual const void HandleEvent(const Event* const event);
	const XMFLOAT2 GetPosition() const;
	~Player();
};