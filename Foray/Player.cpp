#include "stdafx.h"
#include "Player.h"
#include "Constants.h"
#include "../Events/EventHandler.h"
#include "../Events/FireProjectileEvent.h"
#include "../Events/GamepadInputEvent.h"
#include "../Events/KeyDownEvent.h"
#include "../Events/KeyUpEvent.h"
#include "Physics/PhysicsEngine.h"

extern std::unique_ptr<EventHandler> g_eventHandler;
extern std::unique_ptr<PhysicsEngine> g_physicsEngine;

Player::Player(GameObject& gameObject)
	: gameObject{ gameObject }
{
	g_eventHandler->Subscribe(*this);
}

void Player::Update()
{
	// movement
	if (canJump && jumpPressed && verticalVelocity == 0.0f)
	{
		verticalVelocity = PLAYER_JUMP_VELOCITY;
		canJump = false;
		landed = false;
	}
	else if (jumpReleased && verticalVelocity < 0.0f)
	{
		verticalVelocity = 0.0f;
	}

	if (!movingLeft && rightPressed)
	{
		movingLeft = false;
		movingRight = true;
		gameObject.RenderComponent->MirrorHorizontal = false;
	}
	else if (!movingRight && leftPressed)
	{
		movingRight = false;
		movingLeft = true;
		gameObject.RenderComponent->MirrorHorizontal = true;
	}
	else
	{
		movingRight = false;
		movingLeft = false;
		moveFrame = 1;
	}

	// first handle vertical movement
	const auto tmpVertVelocity = verticalVelocity == 0 ? PLAYER_DOWNWARD_ACCELERATION : verticalVelocity;
	const auto proposedVerticalPos = XMFLOAT2{ gameObject.Position.x, gameObject.Position.y + tmpVertVelocity };
	auto verticalCollisionResult = g_physicsEngine->CheckCollisionForPosition(gameObject, 60.0f, 80.0f, proposedVerticalPos);

	if (verticalVelocity < 0.0f && (verticalCollisionResult.GetCollisionDirection() & COLLISION_DIRECTION_TOP) > 0 && verticalCollisionResult.GetCollider()->Type == ColliderType::StaticGeometry)
	{
		auto deltaY = gameObject.Collider->GetRect().top - verticalCollisionResult.GetCollider()->GetRect().bottom;
		gameObject.Translate(XMFLOAT2{ 0.0f, -deltaY });
		verticalVelocity = 0.0f;
	}

	if (verticalVelocity > 0.0f && (verticalCollisionResult.GetCollisionDirection() & COLLISION_DIRECTION_BOTTOM) > 0 && verticalCollisionResult.GetCollider()->Type == ColliderType::StaticGeometry)
	{
		auto deltaY = verticalCollisionResult.GetCollider()->GetRect().top - gameObject.Collider->GetRect().bottom;
		gameObject.Translate(XMFLOAT2{ 0.0f, deltaY });
		verticalVelocity = 0.0f;
		landed = true;
		if (jumpReleased)
			canJump = true;
	}
	else if ((verticalCollisionResult.GetCollisionDirection() & COLLISION_DIRECTION_BOTTOM) == 0 || verticalCollisionResult.GetCollider()->Type != ColliderType::StaticGeometry)
	{
		if (verticalVelocity < PLAYER_MAX_FALL_VELOCITY)
			verticalVelocity += PLAYER_DOWNWARD_ACCELERATION;

		gameObject.Translate(XMFLOAT2{ 0.0f, verticalVelocity });
	}

	// next handle horizontal movement
	if (movingLeft || movingRight)
	{
		const auto horizontalVelocity = movingRight ? PLAYER_HORIZONTAL_MOVE_SPEED : -PLAYER_HORIZONTAL_MOVE_SPEED;
		const auto proposedHorizontalPos = XMFLOAT2{ gameObject.Position.x + horizontalVelocity, gameObject.Position.y };
		auto horizontalCollisionResult = g_physicsEngine->CheckCollisionForPosition(gameObject, 60.0f, 80.0f, proposedHorizontalPos);

		if (movingRight && (horizontalCollisionResult.GetCollisionDirection() & COLLISION_DIRECTION_RIGHT) > 0 && horizontalCollisionResult.GetCollider()->Type == ColliderType::StaticGeometry)
		{
			auto deltaX = horizontalCollisionResult.GetCollider()->GetRect().left - gameObject.Collider->GetRect().right;
			gameObject.Translate(XMFLOAT2{ deltaX, 0.0f });
		}
		else if (movingLeft && (horizontalCollisionResult.GetCollisionDirection() & COLLISION_DIRECTION_LEFT) > 0 && horizontalCollisionResult.GetCollider()->Type == ColliderType::StaticGeometry)
		{
			auto deltaX = gameObject.Collider->GetRect().left - horizontalCollisionResult.GetCollider()->GetRect().right;
			gameObject.Translate(XMFLOAT2{ -deltaX, 0.0f });
		}
		else if (horizontalCollisionResult.GetCollisionDirection() == COLLISION_DIRECTION_NONE || horizontalCollisionResult.GetCollider()->Type != ColliderType::StaticGeometry)
		{
			gameObject.Translate(XMFLOAT2{ horizontalVelocity, 0.0f });
		}
	}

	// animation
	if (canShoot && shootPressed)
	{
		canShoot = false;
		shooting = true;
		shootAnimationTimer = 0.0f;

		const XMFLOAT2 direction = gameObject.RenderComponent->MirrorHorizontal ? XMFLOAT2{ -8.0f, 0.0f } : XMFLOAT2{ 8.0f, 0.0f };
		const XMFLOAT2 spawnPosition = gameObject.RenderComponent->MirrorHorizontal ? gameObject.Position + XMFLOAT2{ -35.0f, 5.0f } : gameObject.Position + XMFLOAT2{ 35.0f, 5.0f };
		std::unique_ptr<Event> e = std::make_unique<FireProjectileEvent>(gameObject.GameObjectId, spawnPosition, direction);
		g_eventHandler->QueueEvent(e);
	}

	if (shooting)
	{
		shootAnimationTimer += UPDATE_FREQUENCY;

		if (shootAnimationTimer >= UPDATE_FREQUENCY * 40)
		{
			shooting = false;
		}
	}

	if (movingLeft || movingRight)
	{
		moveAnimationTimer += UPDATE_FREQUENCY;

		if ((moveFrame == 1 && moveAnimationTimer >= UPDATE_FREQUENCY * 4) || (moveFrame == 2 && moveAnimationTimer >= UPDATE_FREQUENCY * 8) || moveAnimationTimer >= UPDATE_FREQUENCY * 14)
		{
			lastMoveFrame = moveFrame;

			if (moveFrame == 1)
			{
				if (!shooting)
					moveFrame = 2;
				else
					moveFrame = 3;
			}
			else if (moveFrame == 2)
				moveFrame = 3;
			else if (moveFrame == 3)
				moveFrame = 4;
			else if (moveFrame == 4)
			{
				if (lastMoveFrame == 3)
					moveFrame = 5;
				else
					moveFrame = 3;
			}
			else if (moveFrame == 5)
				moveFrame = 4;

			moveAnimationTimer = 0.0f;
		}
	}
	else
		moveAnimationTimer = 0.0f;

	// update texture
	unsigned int textureId;
	if (verticalVelocity != 0.0f)
	{
		if (shooting)
		{
			textureId = 10;
		}
		else
		{
			textureId = 9;
		}
	}
	else
	{
		if (moveFrame == 1)
		{
			if (shooting)
				textureId = 5;
			else
				textureId = 0;
		}
		else if (moveFrame == 2)
			textureId = 1;
		else if (moveFrame == 3)
		{
			if (shooting)
				textureId = 6;
			else
				textureId = 2;
		}
		else if (moveFrame == 4)
		{
			if (shooting)
				textureId = 7;
			else
				textureId = 3;
		}
		else if (moveFrame == 5)
		{
			if (shooting)
				textureId = 8;
			else
				textureId = 4;
		}
	}

	gameObject.RenderComponent->TextureId = textureId;
}

const void Player::HandleEvent(const Event* const event)
{
	const auto type = event->Type;
	switch (type)
	{
		case EventType::KeyDown:
		{
			const auto derivedEvent = (KeyDownEvent*)event;

			if (derivedEvent->CharCode == VK_SPACE)
			{
				jumpPressed = true;
				jumpReleased = false;
			}
			else if (derivedEvent->CharCode == VK_RIGHT)
			{
				rightPressed = true;
			}
			else if (derivedEvent->CharCode == VK_LEFT)
			{
				leftPressed = true;
			}
			else if (derivedEvent->CharCode == VK_LCONTROL)
			{
				shootPressed = true;
			}

			break;
		}
		case EventType::KeyUp:
		{
			const auto derivedEvent = (KeyUpEvent*)event;

			if (derivedEvent->CharCode == VK_SPACE)
			{
				jumpPressed = false;
				jumpReleased = true;
				if (landed)
					canJump = true;
			}
			else if (derivedEvent->CharCode == VK_RIGHT)
			{
				rightPressed = false;
			}
			else if (derivedEvent->CharCode == VK_LEFT)
			{
				leftPressed = false;
			}
			else if (derivedEvent->CharCode == VK_LCONTROL)
			{
				canShoot = true;
				shootPressed = false;
			}

			break;
		}
		case EventType::GamepadInput:
		{
			const auto derivedEvent = (GamepadInputEvent*)event;

			if (derivedEvent->InputValue == XINPUT_GAMEPAD_DPAD_LEFT)
			{
				leftPressed = derivedEvent->Pressed;
			}
			else if (derivedEvent->InputValue == XINPUT_GAMEPAD_DPAD_RIGHT)
			{
				rightPressed = derivedEvent->Pressed;
			}
			else if (derivedEvent->InputValue == XINPUT_GAMEPAD_A)
			{
				if (derivedEvent->Pressed)
				{
					jumpPressed = true;
					jumpReleased = false;
				}
				else
				{
					jumpPressed = false;
					jumpReleased = true;
					if (landed)
						canJump = true;
				}
			}
			else if (derivedEvent->InputValue == VK_PAD_RTRIGGER)
			{
				if (derivedEvent->Pressed)
				{
					shootPressed = true;
				}
				else
				{
					canShoot = true;
					shootPressed = false;
				}
			}
			break;
		}
	}
}

Player::~Player()
{
	g_eventHandler->Unsubscribe(*this);
}