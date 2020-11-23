#pragma once

#include "UIComponent.h"

class UIEditorToolButtonGroup;

class UIEditorToolButton : public UIComponent
{
private:
	UIEditorToolButtonGroup* parent;
	const std::wstring text;
	ComPtr<ID2D1RectangleGeometry> bodyGeometry;
	ID2D1SolidColorBrush* buttonBrush{ nullptr };
	ID2D1SolidColorBrush* pressedButtonBrush{ nullptr };
	ID2D1SolidColorBrush* buttonBorderBrush{ nullptr };
	ID2D1SolidColorBrush* buttonTextBrush{ nullptr };
	ComPtr<IDWriteTextLayout> buttonTextLayout;
	

public:
	bool IsSelected{ false };

	UIEditorToolButton(UIComponentArgs uiComponentArgs, const wchar_t* text, UIEditorToolButtonGroup* parent);
	void Initialize(
		ID2D1SolidColorBrush* buttonBrush,
		ID2D1SolidColorBrush* pressedButtonBrush,
		ID2D1SolidColorBrush* buttonBorderBrush,
		ID2D1SolidColorBrush* buttonTextBrush,
		IDWriteTextFormat* buttonTextFormat);
	void Draw() override;
	const bool HandleEvent(const Event* const event) override;
	const std::wstring GetText() { return text; }
};

