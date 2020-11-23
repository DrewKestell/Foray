#include "stdafx.h"
#include "Utility.h"

extern float g_clientWidth;
extern float g_clientHeight;
extern XMMATRIX g_projectionTransform;

std::wstring Utility::s2ws(const std::string& str)
{
	if (str.empty())
		return std::wstring();
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);

	return wstrTo;
}

const bool Utility::IsOffScreen(const XMFLOAT2 playerPos, const D2D1_RECT_F rect)
{
	return rect.right < playerPos.x - (g_clientWidth / 2)
		|| rect.bottom < playerPos.y - (g_clientHeight / 2)
		|| rect.left > playerPos.x + (g_clientWidth / 2)
		|| rect.top > playerPos.y + (g_clientHeight / 2);
}

const bool Utility::DetectClick(const float topLeftX, const float topLeftY, const float bottomRightX, const float bottomRightY, const float mousePosX, const float mousePosY)
{
	return mousePosX >= topLeftX && mousePosX <= bottomRightX && mousePosY >= topLeftY && mousePosY <= bottomRightY;
}

const XMFLOAT2 Utility::ConvertToWorldSpace(const XMFLOAT2 pos)
{
	XMFLOAT3 position{ pos.x, pos.y, 0.0f };
	FXMVECTOR v = XMLoadFloat3(&position);
	CXMMATRIX view = XMMatrixIdentity();
	CXMMATRIX world = XMMatrixIdentity();

	auto res = XMVector3Unproject(v, 0.0f, 0.0f, g_clientWidth, g_clientHeight, 0.0f, 1000.0f, g_projectionTransform, view, world);
	XMFLOAT3 vec;
	XMStoreFloat3(&vec, res);

	return XMFLOAT2{ vec.x, vec.y };
}