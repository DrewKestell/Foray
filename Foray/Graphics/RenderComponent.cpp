#include "../stdafx.h"
#include "RenderComponent.h"

extern float g_clientWidth;
extern float g_clientHeight;
extern XMMATRIX g_projectionTransform;
extern XMMATRIX g_viewTransform;

RenderComponent::RenderComponent(
	const unsigned int textureId,
	const unsigned int zIndex,
	const XMFLOAT2 position,
	const float width,
	const float height)
	: TextureId{ textureId },
	  ZIndex{ zIndex },
	  Width{ width },
	  Height{ height }
{
	SetPosition(position);
}

void RenderComponent::SetPosition(const XMFLOAT2 newPos)
{
	XMFLOAT3 pos{ newPos.x, newPos.y, 0.0f };
	FXMVECTOR v = XMLoadFloat3(&pos);
	CXMMATRIX view = XMMatrixIdentity();
	CXMMATRIX world = XMMatrixIdentity();

	auto res = XMVector3Unproject(v, 0.0f, 0.0f, g_clientWidth, g_clientHeight, 0.0f, 1000.0f, g_projectionTransform, view, world);
	XMFLOAT3 vec;
	XMStoreFloat3(&vec, res);

	Position = XMFLOAT2{ vec.x, vec.y };
}