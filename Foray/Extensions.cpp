#include "stdafx.h"
#include "Extensions.h"

DirectX::XMFLOAT2 operator+ (const DirectX::XMFLOAT2& l, const DirectX::XMFLOAT2& r)
{
	return XMFLOAT2{ l.x + r.x, l.y + r.y };
}

void operator+= (DirectX::XMFLOAT2& l, const DirectX::XMFLOAT2& r)
{
	l.x += r.x;
	l.y += r.y;
}