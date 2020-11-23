#pragma once

#include "UIComponent.h"
#include "../GameObject.h"

class UIEditorTexturePicker : public UIComponent
{
private:
	unsigned int activeTextureId{ 12 };
	GameObject* gameObject;
	ComPtr<ID2D1RectangleGeometry> borderGeometry;
	ID2D1SolidColorBrush* borderBrush;

public:
	UIEditorTexturePicker(UIComponentArgs uiComponentArgs);
	void Initialize(GameObject* gameObject, ID2D1SolidColorBrush* borderBrush);
	void Draw() override;
	const bool HandleEvent(const Event* const event) override;
	const unsigned int GetActiveTextureId() { return activeTextureId; }

};

