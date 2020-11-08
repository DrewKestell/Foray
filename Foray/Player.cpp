#include "stdafx.h"
#include "Player.h"
#include "Constants.h"
#include "../Events/EventHandler.h"
#include "../Events/ChangeActiveLayerEvent.h"
#include "../Events/FireProjectileEvent.h"
#include "../Events/GamepadInputEvent.h"
#include "../Events/KeyDownEvent.h"
#include "../Events/KeyUpEvent.h"
#include "Physics/PhysicsEngine.h"

extern XMMATRIX g_projectionTransform;
extern float g_clientWidth;
extern float g_clientHeight;
extern unsigned int g_colliderId;
extern std::unique_ptr<EventHandler> g_eventHandler;
extern std::unique_ptr<PhysicsEngine> g_physicsEngine;

Player::Player()
	: collider{ std::make_unique<Collider>(g_colliderId++, D2D1::RectF(position.x - 30.0f, position.y - 40.0f, position.x + 30.0f, position.y + 40.0f), this, ColliderType::Player) }
{
	g_eventHandler->Subscribe(*this);
	g_physicsEngine->RegisterCollider(collider.get());
}	

void Player::Initialize(
	ID3D11VertexShader* vertexShader,
	ID3D11PixelShader* pixelShader,
	const BYTE* vertexShaderBuffer,
	const int vertexShaderSize,
	ID3D11Device* device)
{
	this->vertexShader = vertexShader;
	this->pixelShader = pixelShader;
	this->vertexShaderBuffer = vertexShaderBuffer;
	this->vertexShaderSize = vertexShaderSize;
	this->device = device;

	CreateDDSTextureFromFile(device, L"./Sprites/megaman_moveTexture_frame1.DDS", nullptr, moveTexture_frame1.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"./Sprites/megaman_moveTexture_frame2.DDS", nullptr, moveTexture_frame2.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"./Sprites/megaman_moveTexture_frame3.DDS", nullptr, moveTexture_frame3.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"./Sprites/megaman_moveTexture_frame4.DDS", nullptr, moveTexture_frame4.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"./Sprites/megaman_moveTexture_frame5.DDS", nullptr, moveTexture_frame5.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"./Sprites/megaman_moveShootTexture_frame1.DDS", nullptr, moveShootTexture_frame1.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"./Sprites/megaman_moveShootTexture_frame3.DDS", nullptr, moveShootTexture_frame3.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"./Sprites/megaman_moveShootTexture_frame4.DDS", nullptr, moveShootTexture_frame4.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"./Sprites/megaman_moveShootTexture_frame5.DDS", nullptr, moveShootTexture_frame5.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"./Sprites/megaman_jumpTexture.DDS", nullptr, jumpTexture.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"./Sprites/megaman_jumpShootTexture.DDS", nullptr, jumpShootTexture.ReleaseAndGetAddressOf());
}

void Player::Translate(const XMFLOAT2 vector)
{
	position += vector;
	collider->SetRect(D2D1::RectF(position.x - 30.0f, position.y - 40.0f, position.x + 30.0f, position.y + 40.0f));
}

const CollisionResult Player::CheckCollisionForPosition(const XMFLOAT2 proposedPos) const
{
	const auto proposedColliderPos = D2D1::RectF(proposedPos.x - 30.0f, proposedPos.y - 40.0f, proposedPos.x + 30.0f, proposedPos.y + 40.0f);
	const auto proposedCollider = std::make_unique<Collider>(collider.get()->GetId(), proposedColliderPos, nullptr, ColliderType::Player);
	
	return g_physicsEngine->CheckCollision(proposedCollider.get());
}

void Player::Update()
{
	if (activeLayer == Layer::Game)
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
			mirrorHorizontal = false;
		}
		else if (!movingRight && leftPressed)
		{
			movingRight = false;
			movingLeft = true;
			mirrorHorizontal = true;
		}
		else
		{
			movingRight = false;
			movingLeft = false;
			moveFrame = 1;
		}

		// first handle vertical movement
		const auto tmpVertVelocity = verticalVelocity == 0 ? PLAYER_DOWNWARD_ACCELERATION : verticalVelocity;
		const auto proposedVerticalPos = XMFLOAT2{ position.x, position.y + tmpVertVelocity };
		auto verticalCollisionResult = CheckCollisionForPosition(proposedVerticalPos);

		if (verticalVelocity < 0.0f && (verticalCollisionResult.GetCollisionDirection() & COLLISION_DIRECTION_TOP) > 0 && verticalCollisionResult.GetCollider()->GetColliderType() == ColliderType::StaticGeometry)
		{
			auto deltaY = collider->GetRect().top - verticalCollisionResult.GetCollider()->GetRect().bottom;
			Translate(XMFLOAT2{ 0.0f, -deltaY });
			verticalVelocity = 0.0f;
		}

		if (verticalVelocity > 0.0f && (verticalCollisionResult.GetCollisionDirection() & COLLISION_DIRECTION_BOTTOM) > 0 && verticalCollisionResult.GetCollider()->GetColliderType() == ColliderType::StaticGeometry)
		{
			auto deltaY = verticalCollisionResult.GetCollider()->GetRect().top - collider->GetRect().bottom;
			Translate(XMFLOAT2{ 0.0f, deltaY });
			verticalVelocity = 0.0f;
			landed = true;
			if (jumpReleased)
				canJump = true;
		}
		else if ((verticalCollisionResult.GetCollisionDirection() & COLLISION_DIRECTION_BOTTOM) == 0 || verticalCollisionResult.GetCollider()->GetColliderType() != ColliderType::StaticGeometry)
		{
			if (verticalVelocity < PLAYER_MAX_FALL_VELOCITY)
				verticalVelocity += PLAYER_DOWNWARD_ACCELERATION;

			Translate(XMFLOAT2{ 0.0f, verticalVelocity });
		}
		

		// next handle horizontal movement
		if (movingLeft || movingRight)
		{
			const auto horizontalVelocity = movingRight ? PLAYER_HORIZONTAL_MOVE_SPEED : -PLAYER_HORIZONTAL_MOVE_SPEED;
			const auto proposedHorizontalPos = XMFLOAT2{ position.x + horizontalVelocity, position.y };
			auto horizontalCollisionResult = CheckCollisionForPosition(proposedHorizontalPos);

			if (movingRight && (horizontalCollisionResult.GetCollisionDirection() & COLLISION_DIRECTION_RIGHT) > 0 && horizontalCollisionResult.GetCollider()->GetColliderType() == ColliderType::StaticGeometry)
			{
				auto deltaX = horizontalCollisionResult.GetCollider()->GetRect().left - collider->GetRect().right;
				Translate(XMFLOAT2{ deltaX, 0.0f });
			}
			else if (movingLeft && (horizontalCollisionResult.GetCollisionDirection() & COLLISION_DIRECTION_LEFT) > 0 && horizontalCollisionResult.GetCollider()->GetColliderType() == ColliderType::StaticGeometry)
			{
				auto deltaX = collider->GetRect().left - horizontalCollisionResult.GetCollider()->GetRect().right;
				Translate(XMFLOAT2{ -deltaX, 0.0f });
			}
			else if (horizontalCollisionResult.GetCollisionDirection() == COLLISION_DIRECTION_NONE || horizontalCollisionResult.GetCollider()->GetColliderType() != ColliderType::StaticGeometry)
			{
				Translate(XMFLOAT2{ horizontalVelocity, 0.0f });
			}
		}

		// animation
		if (canShoot && shootPressed)
		{
			canShoot = false;
			shooting = true;
			shootAnimationTimer = 0.0f;

			const XMFLOAT2 direction = mirrorHorizontal ? XMFLOAT2{ -8.0f, 0.0f } : XMFLOAT2{ 8.0f, 0.0f };
			const XMFLOAT2 spawnPosition = mirrorHorizontal ? position + XMFLOAT2{ -35.0f, 5.0f } : position + XMFLOAT2{ 35.0f, 5.0f };
			std::unique_ptr<Event> e = std::make_unique<FireProjectileEvent>(GameObjectId, spawnPosition, direction);
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
	}
}

void Player::Draw(ID3D11DeviceContext* d3dContext)
{
	if (activeLayer == Layer::Game)
	{
		XMFLOAT3 pos{ position.x, position.y, 0.0f };
		FXMVECTOR v = XMLoadFloat3(&pos);
		CXMMATRIX view = XMMatrixIdentity();
		CXMMATRIX world = XMMatrixIdentity();

		ID3D11ShaderResourceView* texture;
		if (verticalVelocity != 0.0f)
		{
			if (shooting)
			{
				texture = jumpShootTexture.Get();
			}
			else
			{
				texture = jumpTexture.Get();
			}
		}
		else
		{
			if (moveFrame == 1)
			{
				if (shooting)
					texture = moveShootTexture_frame1.Get();
				else
					texture = moveTexture_frame1.Get();
			}
			else if (moveFrame == 2)
				texture = moveTexture_frame2.Get();
			else if (moveFrame == 3)
			{
				if (shooting)
					texture = moveShootTexture_frame3.Get();
				else
					texture = moveTexture_frame3.Get();
			}
			else if (moveFrame == 4)
			{
				if (shooting)
					texture = moveShootTexture_frame4.Get();
				else
					texture = moveTexture_frame4.Get();
			}
			else if (moveFrame == 5)
			{
				if (shooting)
					texture = moveShootTexture_frame5.Get();
				else
					texture = moveTexture_frame5.Get();
			}
		}

		auto res = XMVector3Unproject(v, 0.0f, 0.0f, g_clientWidth, g_clientHeight, 0.0f, 1000.0f, g_projectionTransform, view, world);
		XMFLOAT3 vec;
		XMStoreFloat3(&vec, res);
		sprite = std::make_unique<Sprite>(vertexShader, pixelShader, texture, vertexShaderBuffer, vertexShaderSize, device, vec.x, vec.y, 95.0f, 80.0f, 3);

		sprite->Draw(d3dContext, mirrorHorizontal);
	}
}

const void Player::HandleEvent(const Event* const event)
{
	const auto type = event->type;
	switch (type)
	{
		case EventType::ChangeActiveLayer:
		{
			const auto derivedEvent = (ChangeActiveLayerEvent*)event;

			activeLayer = derivedEvent->layer;

			break;
		}
		case EventType::KeyDown:
		{
			const auto derivedEvent = (KeyDownEvent*)event;

			if (derivedEvent->charCode == VK_SPACE)
			{
				jumpPressed = true;
				jumpReleased = false;
			}
			else if (derivedEvent->charCode == VK_RIGHT)
			{
				rightPressed = true;
			}
			else if (derivedEvent->charCode == VK_LEFT)
			{
				leftPressed = true;
			}

			break;
		}
		case EventType::KeyUp:
		{
			const auto derivedEvent = (KeyUpEvent*)event;

			if (derivedEvent->charCode == VK_SPACE)
			{
				jumpPressed = false;
				jumpReleased = true;
				if (landed)
					canJump = true;
			}
			else if (derivedEvent->charCode == VK_RIGHT)
			{
				rightPressed = false;
			}
			else if (derivedEvent->charCode == VK_LEFT)
			{
				leftPressed = false;
			}

			break;
		}
		case EventType::GamepadInput:
		{
			const auto derivedEvent = (GamepadInputEvent*)event;

			if (derivedEvent->inputValue == XINPUT_GAMEPAD_DPAD_LEFT)
			{
				leftPressed = derivedEvent->pressed;
			}
			else if (derivedEvent->inputValue == XINPUT_GAMEPAD_DPAD_RIGHT)
			{
				rightPressed = derivedEvent->pressed;
			}
			else if (derivedEvent->inputValue == XINPUT_GAMEPAD_A)
			{
				if (derivedEvent->pressed)
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
			else if (derivedEvent->inputValue == VK_PAD_RTRIGGER)
			{
				if (derivedEvent->pressed)
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

const void Player::OnCollision(CollisionResult collisionResult)
{
	/*if ((collisionResult.GetCollisionDirection() & COLLISION_DIRECTION_BOTTOM) > 0)
	{
		
	}
	if ((collisionResult.GetCollisionDirection() & COLLISION_DIRECTION_RIGHT) > 0)
	{
		auto deltaX = collider->GetRect().right - collisionResult.GetCollider()->GetRect().left;
		Translate(XMFLOAT2{ 0.0f, deltaX });
		movingRight = false;
	}*/
}

Player::~Player()
{
	g_eventHandler->Unsubscribe(*this);
	g_physicsEngine->UnregisterCollider(collider.get());
}