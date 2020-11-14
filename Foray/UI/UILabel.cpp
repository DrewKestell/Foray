#include "../stdafx.h"
#include "UILabel.h"
#include "../Events/ChangeActiveLayerEvent.h"

UILabel::UILabel(
	UIComponentArgs uiComponentArgs)
	: UIComponent(uiComponentArgs)
{
}

void UILabel::Initialize(ID2D1SolidColorBrush* textBrush, IDWriteTextFormat* textFormat)
{
	this->textBrush = textBrush;
	this->textFormat = textFormat;

	CreateTextLayout();
}

void UILabel::Draw()
{
	if (!isVisible) return;

	const auto position = GetWorldPosition();
	deviceResources->GetD2DDeviceContext()->DrawTextLayout(D2D1::Point2F(position.x, position.y), textLayout.Get(), textBrush);
}

void UILabel::SetText(const wchar_t* text)
{
	this->text = text;

	CreateTextLayout();
}

void UILabel::CreateTextLayout()
{
	const auto hr = deviceResources->GetWriteFactory()->CreateTextLayout(
		this->text.c_str(),
		static_cast<unsigned int>(this->text.size()),
		textFormat,
		FLT_MAX,
		FLT_MAX,
		textLayout.ReleaseAndGetAddressOf()
	);

	if (FAILED(hr))
		throw new std::exception("Failed to create text layout.");
}

const void UILabel::HandleEvent(const Event* const event)
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
	}
}