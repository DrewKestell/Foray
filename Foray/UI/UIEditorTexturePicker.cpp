#include "../stdafx.h"
#include "UIEditorTexturePicker.h"
#include "../Events/MouseEvent.h"
#include "../Events/ChangeActiveLayerEvent.h"

UIEditorTexturePicker::UIEditorTexturePicker(UIComponentArgs uiComponentArgs)
	: UIComponent(uiComponentArgs)
{
}

void UIEditorTexturePicker::Initialize(GameObject* gameObject, ID2D1SolidColorBrush* borderBrush)
{
	this->gameObject = gameObject;
	this->borderBrush = borderBrush;

	const auto position = GetWorldPosition();
	deviceResources->GetD2DFactory()->CreateRectangleGeometry(D2D1_RECT_F{ position.x, position.y, position.x + 52.0f, position.y + 52.0f }, borderGeometry.ReleaseAndGetAddressOf());
}

void UIEditorTexturePicker::Draw()
{
	if (!isVisible) return;

	const auto d2dDeviceContext = deviceResources->GetD2DDeviceContext();
	d2dDeviceContext->DrawGeometry(borderGeometry.Get(), borderBrush, 2.0f);
}

const void UIEditorTexturePicker::HandleEvent(const Event* const event)
{
	// first pass the event to UIComponent base so it can reset localPosition based on new client dimensions
	UIComponent::HandleEvent(event);

	const auto type = event->Type;
	switch (type)
	{
		case EventType::LeftMouseDown:
		{
			const auto mouseDownEvent = (MouseEvent*)event;



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
			deviceResources->GetD2DFactory()->CreateRectangleGeometry(D2D1_RECT_F{ position.x, position.y, position.x + 52.0f, position.y + 52.0f }, borderGeometry.ReleaseAndGetAddressOf());

			gameObject->RenderComponent->SetPosition(XMFLOAT2{ position.x + 26.0f, position.y + 26.0f });

			break;
		}
		case EventType::ReorderUIComponents:
		{
			if (gameObject)
				gameObject->RenderComponent->Visible = isVisible;

			break;
		}
	}
}