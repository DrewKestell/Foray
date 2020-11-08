#include "stdafx.h"
#include "Extensions.h"

XMFLOAT2 operator+ (const XMFLOAT2& l, const XMFLOAT2& r)
{
	return XMFLOAT2{ l.x + r.x, l.y + r.y };
}

void operator+= (XMFLOAT2& l, const XMFLOAT2& r)
{
	l.x += r.x;
	l.y += r.y;
}