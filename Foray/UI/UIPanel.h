#pragma once

#include "UIComponent.h"
#include "../Events/EventHandler.h"
#include "../Events/Observer.h"
#include "../Events/Event.h"

static constexpr auto HEADER_HEIGHT = 20.0f;

class UIPanel : public UIComponent
{
	const bool isDraggable;
	const float width;
	const float height;
	const WPARAM showKey;
	ID2D1SolidColorBrush* headerBrush{ nullptr };
	ID2D1SolidColorBrush* bodyBrush{ nullptr };
	ID2D1SolidColorBrush* borderBrush{ nullptr };
	ComPtr<ID2D1RoundedRectangleGeometry> headerGeometry;
	ComPtr<ID2D1RoundedRectangleGeometry> bodyGeometry;
	bool isActive{ false };
	bool isDragging{ false };
	float lastDragX{ 0.0f };
	float lastDragY{ 0.0f };

	void SetChildrenAsVisible(UIComponent* uiComponent);
	void BringToFront(UIComponent* uiComponent);
	void UpdateChildrenPositions(UIComponent* uiComponent, const Event& e);
public:
	UIPanel(
		UIComponentArgs uiComponentArgs,
		const bool isDraggable,
		const float width,
		const float height,
		const WPARAM showKey);
	void Initialize(
		ID2D1SolidColorBrush* headerBrush,
		ID2D1SolidColorBrush* bodyBrush,
		ID2D1SolidColorBrush* borderBrush);
	void Draw() override;
	const void HandleEvent(const Event* const event) override;
	void ToggleVisibility();
	const bool GetIsDragging() const;
};
