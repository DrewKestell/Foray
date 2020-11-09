#include "stdafx.h"
#include "Projectile.h"
#include "Physics/PhysicsEngine.h"
#include "Events/EventHandler.h"
#include "Events/ChangeActiveLayerEvent.h"
#include "Utility.h"

extern XMMATRIX g_projectionTransform;
extern float g_clientWidth;
extern float g_clientHeight;
extern std::unique_ptr<EventHandler> g_eventHandler;
extern std::unique_ptr<PhysicsEngine> g_physicsEngine;
extern unsigned int g_colliderId;

Projectile::Projectile(const unsigned int ownedId, const XMFLOAT2 position, const XMFLOAT2 velocity)
	: ownerId{ ownerId },
	  position{ position },
	  velocity{ velocity },
	  collider{ std::make_unique<Collider>(g_colliderId++, D2D1::RectF(position.x - 16.0f, position.y - 12.0f, position.x + 16.0f, position.y + 12.0f), this, ColliderType::Projectile) }
{
	g_eventHandler->Subscribe(*this);
	g_physicsEngine->RegisterCollider(collider.get());
}

void Projectile::Initialize(
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

	XMFLOAT3 pos{ position.x, position.y, 0.0f };
	FXMVECTOR v = XMLoadFloat3(&pos);
	CXMMATRIX view = XMMatrixIdentity();
	CXMMATRIX world = XMMatrixIdentity();

	CreateDDSTextureFromFile(device, L"./Graphics/Textures/bullet.DDS", nullptr, bulletTexture.ReleaseAndGetAddressOf());

	auto res = XMVector3Unproject(v, 0.0f, 0.0f, g_clientWidth, g_clientHeight, 0.0f, 1000.0f, g_projectionTransform, view, world);
	XMFLOAT3 vec;
	XMStoreFloat3(&vec, res);
	sprite = std::make_unique<Sprite>(vertexShader, pixelShader, bulletTexture.Get(), vertexShaderBuffer, vertexShaderSize, device, vec.x, vec.y, 16.0f, 12.0f, 3, false);
}

void Projectile::Translate(const XMFLOAT2 vector)
{
	position += vector;
	collider->SetRect(D2D1::RectF(position.x - 16.0f, position.y - 12.0f, position.x + 16.0f, position.y + 12.0f));

	XMFLOAT3 pos{ position.x, position.y, 0.0f };
	FXMVECTOR v = XMLoadFloat3(&pos);
	CXMMATRIX view = XMMatrixIdentity();
	CXMMATRIX world = XMMatrixIdentity();

	auto res = XMVector3Unproject(v, 0.0f, 0.0f, g_clientWidth, g_clientHeight, 0.0f, 1000.0f, g_projectionTransform, view, world);
	XMFLOAT3 vec;
	XMStoreFloat3(&vec, res);
	sprite->SetPosition(device, vec.x, vec.y);
}

void Projectile::Update()
{
	Translate(XMFLOAT2{ velocity });
}

const D2D1_RECT_F Projectile::GetBoundingBox() const
{
	return collider->GetRect();
}

const void Projectile::HandleEvent(const Event* const event)
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
	}
}

const void Projectile::OnCollision(CollisionResult collisionResult)
{
	// TODO
}

Projectile::~Projectile()
{
	g_eventHandler->Unsubscribe(*this);
}