#include "../stdafx.h"
#include "UIEditorToolButton.h"
#include "UIEditorToolButtonGroup.h"
#include "../Events/MouseEvent.h"
#include "../Events/ChangeActiveLayerEvent.h"

UIEditorToolButton::UIEditorToolButton(UIComponentArgs uiComponentArgs, const wchar_t* text, UIEditorToolButtonGroup* parent)
	: UIComponent(uiComponentArgs),
	  text{ text },
	  parent{ parent }
{
}

void UIEditorToolButton::Initialize(
	ID2D1SolidColorBrush* buttonBrush,
	ID2D1SolidColorBrush* pressedButtonBrush,
	ID2D1SolidColorBrush* buttonBorderBrush,
	ID2D1SolidColorBrush* buttonTextBrush,
	IDWriteTextFormat* buttonTextFormat)
{
	this->buttonBrush = buttonBrush;
	this->pressedButtonBrush = pressedButtonBrush;
	this->buttonBorderBrush = buttonBorderBrush;
	this->buttonTextBrush = buttonTextBrush;

	const auto width = 20.0f;
	const auto height = 20.0f;

	deviceResources->GetWriteFactory()->CreateTextLayout(
		text.c_str(),
		static_cast<unsigned int>(text.size()),
		buttonTextFormat,
		width,
		height,
		buttonTextLayout.ReleaseAndGetAddressOf()
	);

	const auto position = GetWorldPosition();
	deviceResources->GetD2DFactory()->CreateRectangleGeometry(D2D1_RECT_F{ position.x, position.y, position.x + width, position.y + height }, bodyGeometry.ReleaseAndGetAddressOf());
}

void UIEditorToolButton::Draw()
{
	if (!isVisible) return;

	// Draw Input
	const float borderWeight = IsSelected ? 2.0f : 1.0f;
	ID2D1SolidColorBrush* buttonColor;
	if (IsSelected)
		buttonColor = pressedButtonBrush;
	else
		buttonColor = buttonBrush;

	const auto d2dDeviceContext = deviceResources->GetD2DDeviceContext();

	d2dDeviceContext->FillGeometry(bodyGeometry.Get(), buttonColor);
	d2dDeviceContext->DrawGeometry(bodyGeometry.Get(), buttonBorderBrush, borderWeight);

	// Draw Input Text   
	const auto position = GetWorldPosition();
	d2dDeviceContext->DrawTextLayout(D2D1::Point2F(position.x, position.y), buttonTextLayout.Get(), buttonTextBrush); // (location + 1) looks better
}

const bool UIEditorToolButton::HandleEvent(const Event* const event)
{
	// first pass the event to UIComponent base so it can reset localPosition based on new client dimensions
	UIComponent::HandleEvent(event);

	const auto type = event->Type;
	switch (type)
	{
		case EventType::LeftMouseDown:
		{
			const auto mouseDownEvent = (MouseEvent*)event;

			if (isVisible)
			{
				const auto position = GetWorldPosition();
				if (Utility::DetectClick(position.x, position.y, position.x + 20.0f, position.y + 20.0f, mouseDownEvent->MousePosX, mouseDownEvent->MousePosY))
				{
					parent->DeselectAllButtons();
					IsSelected = true;
					return true;
				}
			}

			break;
		}
		case EventType::ChangeActiveLayer:
		{
			const auto derivedEvent = (ChangeActiveLayerEvent*)event;

			if (derivedEvent->Layer == uiLayer && GetParent() == nullptr)
				isVisible = true;
			else
				isVisible = false;

			break;
		}
		case EventType::WindowResize:
		{
			const auto position = GetWorldPosition();
			deviceResources->GetD2DFactory()->CreateRectangleGeometry(D2D1_RECT_F{ position.x, position.y, position.x + 20.0f, position.y + 20.0f }, bodyGeometry.ReleaseAndGetAddressOf());

			break;
		}
	}

	return false;
}