#include "stdafx.h"
#include "Player.h"
#include "DDSTextureLoader.h"

using namespace DirectX;

extern XMMATRIX g_projectionTransform;
extern float g_clientWidth;
extern float g_clientHeight;

void Player::Initialize(
	ID3D11VertexShader* vertexShader,
	ID3D11PixelShader* pixelShader,
	const BYTE* vertexShaderBuffer,
	const int vertexShaderSize,
	ID3D11Device* device)
{
	DirectX::CreateDDSTextureFromFile(device, L"./Sprites/megaman.DDS", nullptr, texture.ReleaseAndGetAddressOf());

	// create sprite
	XMFLOAT3 pos{ 22.0f + 20.0f, 62.0f + 20.0f, 0.0f };
	FXMVECTOR v = XMLoadFloat3(&pos);
	CXMMATRIX view = XMMatrixIdentity();
	CXMMATRIX world = XMMatrixIdentity();

	auto res = XMVector3Unproject(v, 0.0f, 0.0f, g_clientWidth, g_clientHeight, 0.0f, 1000.0f, g_projectionTransform, view, world);
	XMFLOAT3 vec;
	XMStoreFloat3(&vec, res);
	sprite = std::make_unique<Sprite>(vertexShader, pixelShader, texture.Get(), vertexShaderBuffer, vertexShaderSize, device, vec.x, vec.y, 40, 40, 3);
}

void Player::Draw(ID3D11DeviceContext* d3dContext)
{
	sprite->Draw(d3dContext);
}

const void Player::HandleEvent(const Event* const event)
{
	const auto type = event->type;
	switch (type)
	{
	
	}
}
