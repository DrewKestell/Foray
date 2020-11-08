#pragma once

class Utility
{
public:
	static std::wstring s2ws(const std::string& str);
	static const bool IsOffScreen(const D2D1_RECT_F rect);
};
