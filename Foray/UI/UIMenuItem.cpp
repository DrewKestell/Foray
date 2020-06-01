#include "../stdafx.h"
#include "UIMenuItem.h"
#include "../Events/ChangeActiveLayerEvent.h"
#include "../Utility.h"

UIMenuItem::UIMenuItem(
	UIComponentArgs uiComponentArgs,
	const float width,
	const char* text)
	: UIComponent(uiComponentArgs),
	  width{ width },
	  text{ text }
{
}

void UIMenuItem::Initialize(ID2D1SolidColorBrush* textBrush, IDWriteTextFormat* textFormat)
{
	this->textBrush = textBrush;
	this->textFormat = textFormat;

	CreateTextLayout();
}

void UIMenuItem::Draw()
{
	if (!isVisible) return;

	const auto position = GetWorldPosition();
	deviceResources->GetD2DDeviceContext()->DrawTextLayout(D2D1::Point2F(position.x, position.y), textLayout.Get(), textBrush);
}

void UIMenuItem::CreateTextLayout()
{
	const auto hr = deviceResources->GetWriteFactory()->CreateTextLayout(
		Utility::s2ws(this->text).c_str(),
		static_cast<unsigned int>(this->text.size()),
		textFormat,
		width,
		24.0f,
		textLayout.ReleaseAndGetAddressOf()
	);

	if (FAILED(hr))
		throw new std::exception("Failed to create text layout.");
}

const void UIMenuItem::HandleEvent(const Event* const event)
{
	// first pass the event to UIComponent base so it can reset localPosition based on new client dimensions
	UIComponent::HandleEvent(event);

	const auto type = event->type;
	switch (type)
	{
	case EventType::ChangeActiveLayer:
	{
		const auto derivedEvent = (ChangeActiveLayerEvent*)event;

		if (derivedEvent->layer == uiLayer && GetParent() == nullptr)
			isVisible = true;
		else
			isVisible = false;

		break;
	}
	}
}