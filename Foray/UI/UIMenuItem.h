#pragma once

#include "UIComponent.h"
#include "../Events/Observer.h"
#include "../Events/Event.h"

class UIMenuItem : public UIComponent
{
	const float width;
	ComPtr<IDWriteTextLayout> textLayout;
	ComPtr<IDWriteTextLayout> bulletTextLayout;
	std::string text;
	ID2D1SolidColorBrush* textBrush{ nullptr };
	IDWriteTextFormat* textFormat{ nullptr };
	IDWriteTextFormat* bulletTextFormat{ nullptr };
	bool isActive{ false };

public:
	UIMenuItem(UIComponentArgs uiComponentArgs, const float width, const char* text, const bool isActive = false);
	void Initialize(ID2D1SolidColorBrush* textBrush, IDWriteTextFormat* textFormat, IDWriteTextFormat* bulletTextFormat);
	void Draw() override;
	const void HandleEvent(const Event* const event) override;
	void CreateTextLayouts();
	void SetActive(const bool isActive);
};