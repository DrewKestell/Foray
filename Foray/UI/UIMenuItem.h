#pragma once

#include "UIComponent.h"
#include "../Events/Observer.h"
#include "../Events/Event.h"

class UIMenuItem : public UIComponent
{
	const float width;
	ComPtr<IDWriteTextLayout> textLayout;
	std::string text;
	ID2D1SolidColorBrush* textBrush{ nullptr };
	IDWriteTextFormat* textFormat{ nullptr };
	bool isActive{ false };

public:
	UIMenuItem(UIComponentArgs uiComponentArgs, const float width, const char* text);
	void Initialize(ID2D1SolidColorBrush* textBrush, IDWriteTextFormat* textFormat);
	void Draw() override;
	const void HandleEvent(const Event* const event) override;
	void CreateTextLayout();
};