#pragma once

#include <string>
#include <d2d1_3.h>

class Utility
{
public:
	static std::wstring s2ws(const std::string& str);
	static const bool IsOffScreen(const D2D1_RECT_F rect);
};
