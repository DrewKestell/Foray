#pragma once

#include "UIComponent.h"
#include "../Events/Observer.h"
#include "../Events/Event.h"

class UILabel : public UIComponent
{
	ID2D1SolidColorBrush* textBrush{ nullptr };
	IDWriteTextFormat* textFormat{ nullptr };
	ComPtr<IDWriteTextLayout> textLayout;
	std::wstring text;

public:
	UILabel(UIComponentArgs uiComponentArgs);
	void Initialize(ID2D1SolidColorBrush* textBrush, IDWriteTextFormat* textFormat);
	void Draw() override;
	const bool HandleEvent(const Event* const event) override;
	void SetText(const wchar_t* arr);
	void CreateTextLayout();
};