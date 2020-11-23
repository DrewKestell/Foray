#include "../stdafx.h"
#include "UIButton.h"
#include "../Events/MouseEvent.h"
#include "../Events/ChangeActiveLayerEvent.h"

UIButton::UIButton(
	UIComponentArgs uiComponentArgs,
	const float width,
	const float height,
	const wchar_t* buttonText,
	const std::function<void()> onClick)
	: UIComponent(uiComponentArgs),
	width{ width },
	height{ height },
	buttonText{ buttonText },
	onClick{ onClick }
{
}

void UIButton::Initialize(
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

	deviceResources->GetWriteFactory()->CreateTextLayout(
		buttonText.c_str(),
		static_cast<unsigned int>(buttonText.size()),
		buttonTextFormat,
		width,
		height,
		buttonTextLayout.ReleaseAndGetAddressOf()
	);

	const auto position = GetWorldPosition();
	deviceResources->GetD2DFactory()->CreateRoundedRectangleGeometry(D2D1::RoundedRect(D2D1::RectF(position.x, position.y, position.x + width, position.y + height), 3.0f, 3.0f), buttonGeometry.ReleaseAndGetAddressOf());
}

void UIButton::Draw()
{
	if (!isVisible) return;

	// Draw Input
	const float borderWeight = pressed ? 2.0f : 1.0f;
	ID2D1SolidColorBrush* buttonColor;
	if (pressed)
		buttonColor = pressedButtonBrush;
	else if (!enabled)
		buttonColor = disabledBrush;
	else
		buttonColor = buttonBrush;

	const auto d2dDeviceContext = deviceResources->GetD2DDeviceContext();

	d2dDeviceContext->FillGeometry(buttonGeometry.Get(), buttonColor);
	d2dDeviceContext->DrawGeometry(buttonGeometry.Get(), buttonBorderBrush, borderWeight);

	// Draw Input Text   
	const auto position = GetWorldPosition();
	d2dDeviceContext->DrawTextLayout(D2D1::Point2F(position.x, position.y), buttonTextLayout.Get(), buttonTextBrush);
}

const bool UIButton::HandleEvent(const Event* const event)
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
				if (Utility::DetectClick(position.x, position.y, position.x + width, position.y + height, mouseDownEvent->MousePosX, mouseDownEvent->MousePosY))
				{
					pressed = true;
					onClick();
					return true;
				}
			}

			break;
		}
		case EventType::LeftMouseUp:
		{
			const auto mouseUpEvent = (MouseEvent*)event;

			pressed = false;

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
			deviceResources->GetD2DFactory()->CreateRoundedRectangleGeometry(D2D1::RoundedRect(D2D1::RectF(position.x, position.y, position.x + width, position.y + height), 3.0f, 3.0f), buttonGeometry.ReleaseAndGetAddressOf());

			break;
		}
	}

	return false;
}