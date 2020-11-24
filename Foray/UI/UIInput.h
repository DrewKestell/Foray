#pragma once

#include "UIComponent.h"

class UIInput : public UIComponent
{
	float width;
	float height;
	ID2D1SolidColorBrush* inputBrush{ nullptr };
	ID2D1SolidColorBrush* inputBorderBrush{ nullptr };
	ID2D1SolidColorBrush* inputValueBrush{ nullptr };
	IDWriteTextFormat* inputValueTextFormat{ nullptr };
	ComPtr<IDWriteTextLayout> inputValueTextLayout;
	ComPtr<ID2D1RectangleGeometry> inputGeometry;
	int inputIndex{ 0 };
	wchar_t inputValue[50] = { 0 };
	bool active{ false };

	void CreateTextLayout();

	friend class UIInputGroup;
public:
	UIInput(
		UIComponentArgs uiComponentArgs,
		const float width,
		const float height);
	void Initialize(
		ID2D1SolidColorBrush* inputBrush,
		ID2D1SolidColorBrush* inputBorderBrush,
		ID2D1SolidColorBrush* inputValueBrush,
		IDWriteTextFormat* inputValueTextFormat);
	void Draw() override;
	const bool HandleEvent(const Event* const event) override;
	const wchar_t* GetInputValue() const;
	void ClearInput();
	const bool IsActive() const { return active; }
	void SetActive(const bool active) { this->active = active; }
};
