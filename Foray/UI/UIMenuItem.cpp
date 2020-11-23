#include "../stdafx.h"
#include "UIMenuItem.h"
#include "../Events/ChangeActiveLayerEvent.h"
#include "../Events/GamepadInputEvent.h"
#include "../Events/KeyDownEvent.h"
#include "../Constants.h"

UIMenuItem::UIMenuItem(
	UIComponentArgs uiComponentArgs,
	const char* text,
	const std::function<void()> onActivate,
	const bool isActive)
	: UIComponent(uiComponentArgs),
	  text{ text },
	  onActivate{ onActivate },
	  isActive{ isActive }
{
}

void UIMenuItem::Initialize(ID2D1SolidColorBrush* textBrush, IDWriteTextFormat* textFormat, IDWriteTextFormat* bulletTextFormat)
{
	this->textBrush = textBrush;
	this->textFormat = textFormat;
	this->bulletTextFormat = bulletTextFormat;

	CreateTextLayouts();
}

void UIMenuItem::Draw()
{
	if (!isVisible) return;

	const auto position = GetWorldPosition();
	deviceResources->GetD2DDeviceContext()->DrawTextLayout(D2D1::Point2F(position.x, position.y), textLayout.Get(), textBrush);

	if (isActive)
		deviceResources->GetD2DDeviceContext()->DrawTextLayout(D2D1::Point2F(position.x - 19.f, position.y - 3.5f), bulletTextLayout.Get(), textBrush);
}

void UIMenuItem::CreateTextLayouts()
{
	auto hr = deviceResources->GetWriteFactory()->CreateTextLayout(
		Utility::s2ws(this->text).c_str(),
		static_cast<unsigned int>(this->text.size()),
		textFormat,
		FLT_MAX,
		FLT_MAX,
		textLayout.ReleaseAndGetAddressOf()
	);

	if (FAILED(hr))
		throw new std::exception("Failed to create text layout.");

	hr = deviceResources->GetWriteFactory()->CreateTextLayout(
		BULLET_TEXT,
		1,
		bulletTextFormat,
		FLT_MAX,
		FLT_MAX,
		bulletTextLayout.ReleaseAndGetAddressOf()
	);

	if (FAILED(hr))
		throw new std::exception("Failed to create text layout.");
}

const bool UIMenuItem::HandleEvent(const Event* const event)
{
	// first pass the event to UIComponent base so it can reset localPosition based on new client dimensions
	UIComponent::HandleEvent(event);

	const auto type = event->Type;
	switch (type)
	{
		case EventType::ChangeActiveLayer:
		{
			const auto derivedEvent = (ChangeActiveLayerEvent*)event;

			if (derivedEvent->Layer == uiLayer && GetParent() == nullptr)
				isVisible = true;
			else
				isVisible = false;

			break;
		}
		case EventType::GamepadInput:
		{
			const auto derivedEvent = (GamepadInputEvent*)event;

			if (isVisible && isActive && derivedEvent->InputValue == XINPUT_GAMEPAD_A && derivedEvent->Pressed)
				onActivate();

			break;
		}
		case EventType::KeyDown:
		{
			const auto derivedEvent = (KeyDownEvent*)event;

			if (isVisible && isActive && derivedEvent->CharCode == VK_RETURN)
				onActivate();

			break;
		}
	}

	return false;
}

const bool UIMenuItem::IsActive()
{
	return isActive;
}

void UIMenuItem::SetActive(const bool isActive)
{
	this->isActive = isActive;
}