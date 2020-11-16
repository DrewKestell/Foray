#include "stdafx.h"
#include "Utility.h"

extern float g_clientWidth;
extern float g_clientHeight;

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