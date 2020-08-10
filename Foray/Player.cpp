#include "stdafx.h"
#include "Player.h"
#include <Windows.h>
#include <Xinput.h>
#include "Constants.h"
#include "DDSTextureLoader.h"
#include "../Events/ChangeActiveLayerEvent.h"
#include "../Events/GamepadInputEvent.h"
#include "Physics/PhysicsEngine.h"
#include <iostream>

using namespace DirectX;

extern XMMATRIX g_projectionTransform;
extern float g_clientWidth;
extern float g_clientHeight;
extern std::unique_ptr<PhysicsEngine> g_physicsEngine;

Player::Player(EventHandler& eventHandler)
	: eventHandler{ eventHandler },
	  collider{ std::make_unique<Collider>(eventHandler, D2D1::RectF(position.x, position.y, position.x + 80.0f, position.y + 80.0f)) }
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

void Player::Update()
{
	if (activeLayer == Layer::Game)
	{
		if (movingRight)
		{
			position += XMFLOAT2{ PLAYER_HORIZONTAL_MOVE_SPEED, 0.0f };
		}
		else if (movingLeft)
		{
			position += XMFLOAT2{ -PLAYER_HORIZONTAL_MOVE_SPEED, 0.0f };
		}

		if (movingLeft || movingRight)
		{
			moveAnimationTimer += UPDATE_FREQUENCY;

			if ((moveFrame == 1 && moveAnimationTimer >= UPDATE_FREQUENCY * 2) || (moveFrame == 2 && moveAnimationTimer >= UPDATE_FREQUENCY * 4) || moveAnimationTimer >= UPDATE_FREQUENCY * 7)
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

			if (!movingLeft && !movingRight)
				moveFrame = 1;

			break;
		}
	}
}

const void Player::OnCollision(Collider* collider)
{
	std::cout << "Collision detected!" << std::endl;
}

Player::~Player()
{
	eventHandler.Unsubscribe(*this);
	g_physicsEngine->UnregisterCollider(collider.get());
}