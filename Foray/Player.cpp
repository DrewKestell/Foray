#include "stdafx.h"
#include "Player.h"
#include <Windows.h>
#include <Xinput.h>
#include "Constants.h"
#include "DDSTextureLoader.h"
#include "../Events/ChangeActiveLayerEvent.h"
#include "../Events/GamepadInputEvent.h"
#include "../Events/KeyDownEvent.h"
#include "../Events/KeyUpEvent.h"
#include "Physics/PhysicsEngine.h"
#include <iostream>

using namespace DirectX;

extern XMMATRIX g_projectionTransform;
extern float g_clientWidth;
extern float g_clientHeight;
extern unsigned int g_colliderId;
extern std::unique_ptr<PhysicsEngine> g_physicsEngine;

Player::Player(EventHandler& eventHandler)
	: eventHandler{ eventHandler },
	  collider{ std::make_unique<Collider>(eventHandler, g_colliderId++, D2D1::RectF(position.x - 40.0f, position.y - 40.0f, position.x + 40.0f, position.y + 40.0f), this) }
{
	eventHandler.Subscribe(*this);
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

	DirectX::CreateDDSTextureFromFile(device, L"./Sprites/megaman_moveTexture_frame1.DDS", nullptr, moveTexture_frame1.ReleaseAndGetAddressOf());
	DirectX::CreateDDSTextureFromFile(device, L"./Sprites/megaman_moveTexture_frame2.DDS", nullptr, moveTexture_frame2.ReleaseAndGetAddressOf());
	DirectX::CreateDDSTextureFromFile(device, L"./Sprites/megaman_moveTexture_frame3.DDS", nullptr, moveTexture_frame3.ReleaseAndGetAddressOf());
	DirectX::CreateDDSTextureFromFile(device, L"./Sprites/megaman_moveTexture_frame4.DDS", nullptr, moveTexture_frame4.ReleaseAndGetAddressOf());
	DirectX::CreateDDSTextureFromFile(device, L"./Sprites/megaman_moveTexture_frame5.DDS", nullptr, moveTexture_frame5.ReleaseAndGetAddressOf());
}

void Player::Translate(const XMFLOAT2 vector)
{
	position += vector;
	collider->SetRect(D2D1::RectF(position.x - 40.0f, position.y - 40.0f, position.x + 40.0f, position.y + 40.0f));
}

const CollisionResult Player::CheckCollisionForPosition(const XMFLOAT2 proposedPos) const
{
	const auto proposedColliderPos = D2D1::RectF(proposedPos.x - 40.0f, proposedPos.y - 40.0f, proposedPos.x + 40.0f, proposedPos.y + 40.0f);
	const auto proposedCollider = std::make_unique<Collider>(eventHandler, collider.get()->GetId(), proposedColliderPos, nullptr);
	
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

		// first handle vertical movement
		const auto tmpVertVelocity = verticalVelocity == 0 ? PLAYER_DOWNWARD_ACCELERATION : verticalVelocity;
		const auto proposedVerticalPos = XMFLOAT2{ position.x, position.y + tmpVertVelocity };
		auto verticalCollisionResult = CheckCollisionForPosition(proposedVerticalPos);

		if (verticalVelocity < 0.0f && (verticalCollisionResult.GetCollisionDirection() & COLLISION_DIRECTION_TOP) > 0)
		{
 			auto deltaY = collider->GetRect().top - verticalCollisionResult.GetCollider()->GetRect().bottom;
			Translate(XMFLOAT2{ 0.0f, -deltaY });
			verticalVelocity = 0.0f;
		}
		else if (verticalVelocity > 0.0f && (verticalCollisionResult.GetCollisionDirection() & COLLISION_DIRECTION_BOTTOM) > 0)
		{
			auto deltaY = verticalCollisionResult.GetCollider()->GetRect().top - collider->GetRect().bottom;
			Translate(XMFLOAT2{ 0.0f, deltaY });
			verticalVelocity = 0.0f;
			landed = true;
			if (jumpReleased)
				canJump = true;
		}
		else if ((verticalCollisionResult.GetCollisionDirection() & COLLISION_DIRECTION_BOTTOM) == 0)
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

			if (movingRight && (horizontalCollisionResult.GetCollisionDirection() & COLLISION_DIRECTION_RIGHT) > 0)
			{
				auto deltaX = horizontalCollisionResult.GetCollider()->GetRect().left - collider->GetRect().right;
				Translate(XMFLOAT2{ deltaX, 0.0f });
			}
			else if (movingLeft && (horizontalCollisionResult.GetCollisionDirection() & COLLISION_DIRECTION_LEFT) > 0)
			{
				auto deltaX = collider->GetRect().left - horizontalCollisionResult.GetCollider()->GetRect().right;
				Translate(XMFLOAT2{ -deltaX, 0.0f });
			}
			else
			{
				Translate(XMFLOAT2{ horizontalVelocity, 0.0f });
			}
		}

		// animation
		if (movingLeft || movingRight)
		{
			moveAnimationTimer += UPDATE_FREQUENCY;

			if ((moveFrame == 1 && moveAnimationTimer >= UPDATE_FREQUENCY * 4) || (moveFrame == 2 && moveAnimationTimer >= UPDATE_FREQUENCY * 8) || moveAnimationTimer >= UPDATE_FREQUENCY * 14)
			{
				lastMoveFrame = moveFrame;

				if (moveFrame == 1)
					moveFrame = 2;
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
		if (moveFrame == 1)
			texture = moveTexture_frame1.Get();
		else if (moveFrame == 2)
			texture = moveTexture_frame2.Get();
		else if (moveFrame == 3)
			texture = moveTexture_frame3.Get();
		else if (moveFrame == 4)
			texture = moveTexture_frame4.Get();
		else if (moveFrame == 5)
			texture = moveTexture_frame5.Get();

		auto res = XMVector3Unproject(v, 0.0f, 0.0f, g_clientWidth, g_clientHeight, 0.0f, 1000.0f, g_projectionTransform, view, world);
		XMFLOAT3 vec;
		XMStoreFloat3(&vec, res);
		sprite = std::make_unique<Sprite>(vertexShader, pixelShader, texture, vertexShaderBuffer, vertexShaderSize, device, vec.x, vec.y, 80.0f, 80.0f, 3);

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
				if (!movingLeft)
				{
					movingRight = true;
					mirrorHorizontal = false;
				}
			}
			else if (derivedEvent->charCode == VK_LEFT)
			{
				if (!movingRight)
				{
					movingLeft = true;
					mirrorHorizontal = true;
				}
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
				movingRight = false;
			}
			else if (derivedEvent->charCode == VK_LEFT)
			{
				movingLeft = false;
			}

			if (!movingLeft && !movingRight)
				moveFrame = 1;

			break;
		}
		case EventType::GamepadInput:
		{
			const auto derivedEvent = (GamepadInputEvent*)event;

			if (derivedEvent->inputValue == XINPUT_GAMEPAD_DPAD_LEFT)
			{
				movingLeft = derivedEvent->pressed;
				mirrorHorizontal = true;
			}
			else if (derivedEvent->inputValue == XINPUT_GAMEPAD_DPAD_RIGHT)
			{
				movingRight = derivedEvent->pressed;
				mirrorHorizontal = false;
			}
			else if (derivedEvent->inputValue == XINPUT_GAMEPAD_A)
			{
				jumpPressed = derivedEvent->pressed;
			}

			if (!movingLeft && !movingRight)
				moveFrame = 1;

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
	eventHandler.Unsubscribe(*this);
	g_physicsEngine->UnregisterCollider(collider.get());
}