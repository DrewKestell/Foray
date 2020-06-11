#pragma once

#include "UIComponent.h"
#include "../Events/Observer.h"
#include "../Events/Event.h"

class UIMenuItem : public UIComponent
{
	ComPtr<IDWriteTextLayout> textLayout;
	ComPtr<IDWriteTextLayout> bulletTextLayout;
	std::string text;
	const std::function<void()> onActivate;
	ID2D1SolidColorBrush* textBrush{ nullptr };
	IDWriteTextFormat* textFormat{ nullptr };
	IDWriteTextFormat* bulletTextFormat{ nullptr };
	bool isActive{ false };

public:
	UIMenuItem(
		UIComponentArgs uiComponentArgs,
		const char* text,
		const std::function<void()> onActivate,
		const bool isActive = false);
	void Initialize(ID2D1SolidColorBrush* textBrush, IDWriteTextFormat* textFormat, IDWriteTextFormat* bulletTextFormat);
	void Draw() override;
	const void HandleEvent(const Event* const event) override;
	void CreateTextLayouts();
	const bool IsActive();
	void SetActive(const bool isActive);
};