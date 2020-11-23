#include "../stdafx.h"
#include "UIPanel.h"
#include "../Events/MouseEvent.h"
#include "../Events/ChangeActiveLayerEvent.h"
#include "../Events/KeyDownEvent.h"

// incoming globals
extern std::unique_ptr<EventHandler> g_eventHandler;
extern unsigned int g_zIndex;

UIPanel::UIPanel(
	UIComponentArgs uiComponentArgs,
	const bool isDraggable,
	const float width,
	const float height,
	const WPARAM showKey)
	: UIComponent(uiComponentArgs),
	isDraggable{ isDraggable },
	width{ width },
	height{ height },
	showKey{ showKey }
{
}

void UIPanel::Initialize(
	ID2D1SolidColorBrush* headerBrush,
	ID2D1SolidColorBrush* bodyBrush,
	ID2D1SolidColorBrush* borderBrush)
{
	this->headerBrush = headerBrush;
	this->bodyBrush = bodyBrush;
	this->borderBrush = borderBrush;

	const auto d2dFactory = deviceResources->GetD2DFactory();
	const auto position = GetWorldPosition();
	float startHeight = position.y;
	if (isDraggable)
	{
		d2dFactory->CreateRoundedRectangleGeometry(D2D1::RoundedRect(D2D1::RectF(position.x, position.y, position.x + width, position.y + HEADER_HEIGHT), 3.0f, 3.0f), headerGeometry.ReleaseAndGetAddressOf());
		startHeight += HEADER_HEIGHT;
	}
	d2dFactory->CreateRoundedRectangleGeometry(D2D1::RoundedRect(D2D1::RectF(position.x, startHeight, position.x + width, startHeight + height), 3.0f, 3.0f), bodyGeometry.ReleaseAndGetAddressOf());

}

void UIPanel::Draw()
{
	if (!isVisible) return;

	const auto d2dDeviceContext = deviceResources->GetD2DDeviceContext();

	// Draw Panel
	const float borderWeight = 2.0f;
	if (isDraggable)
	{
		d2dDeviceContext->FillGeometry(headerGeometry.Get(), headerBrush);
		d2dDeviceContext->DrawGeometry(headerGeometry.Get(), borderBrush, borderWeight);
	}
	d2dDeviceContext->FillGeometry(bodyGeometry.Get(), bodyBrush);
	d2dDeviceContext->DrawGeometry(bodyGeometry.Get(), borderBrush, borderWeight);

	// Draw "X" Button
	const auto position = GetWorldPosition();
	auto startingX = position.x + width - 17.0f;
	auto startingY = position.y + 4.0f;

	d2dDeviceContext->DrawLine(D2D1::Point2F(startingX, startingY), D2D1::Point2F(startingX + 12.0f, startingY + 12.0f), borderBrush, 2.0f);
	d2dDeviceContext->DrawLine(D2D1::Point2F(startingX + 12.0f, startingY), D2D1::Point2F(startingX, startingY + 12.0f), borderBrush, 2.0f);
}

const bool UIPanel::HandleEvent(const Event* const event)
{
	const auto type = event->Type;
	switch (type)
	{
		case EventType::RightMouseDown:
		{
			const auto mouseDownEvent = (MouseEvent*)event;

			if (isVisible)
			{
				const auto position = GetWorldPosition();
				if (Utility::DetectClick(position.x, position.y, position.x + width, position.y + HEADER_HEIGHT + height, mouseDownEvent->MousePosX, mouseDownEvent->MousePosY))
					return true;
			}

			break;
		}
		case EventType::LeftMouseDown:
		{
			const auto mouseDownEvent = (MouseEvent*)event;

			if (isVisible)
			{
				const auto position = GetWorldPosition();
				bool stopEvent{ false };
				if (isDraggable && Utility::DetectClick(position.x, position.y, position.x + width, position.y + HEADER_HEIGHT, mouseDownEvent->MousePosX, mouseDownEvent->MousePosY))
				{
					lastDragX = mouseDownEvent->MousePosX;
					lastDragY = mouseDownEvent->MousePosY;
					isDragging = true;
				}
				if (Utility::DetectClick(position.x, position.y, position.x + width, position.y + HEADER_HEIGHT + height, mouseDownEvent->MousePosX, mouseDownEvent->MousePosY))
				{
					g_zIndex++;
					zIndex = g_zIndex;
					BringToFront(this);

					std::unique_ptr<Event> e = std::make_unique<Event>(EventType::ReorderUIComponents);
					g_eventHandler->QueueEvent(e);

					stopEvent = true;
				}

				const auto closeButtonStartingX = position.x + width - 17.0f;
				const auto closeButtonStartingY = position.y + 4.0f;
				if (Utility::DetectClick(closeButtonStartingX, closeButtonStartingY, closeButtonStartingX + 12.0f, closeButtonStartingY + 12.0f, mouseDownEvent->MousePosX, mouseDownEvent->MousePosY))
				{
					ToggleVisibility();

					stopEvent = true;
				}

				return stopEvent;
			}

			break;
		}
		case EventType::LeftMouseUp:
		{
			const auto mouseUpEvent = (MouseEvent*)event;

			isDragging = false;

			break;
		}
		case EventType::MouseMove:
		{
			const auto mouseMoveEvent = (MouseEvent*)event;

			if (isDragging)
			{
				const auto deltaX = mouseMoveEvent->MousePosX - lastDragX;
				const auto deltaY = mouseMoveEvent->MousePosY - lastDragY;

				Translate(XMFLOAT2{ deltaX, deltaY });

				lastDragX = mouseMoveEvent->MousePosX;
				lastDragY = mouseMoveEvent->MousePosY;

				const auto currentPosition = GetWorldPosition();

				const auto d2dFactory = deviceResources->GetD2DFactory();
				d2dFactory->CreateRoundedRectangleGeometry(D2D1::RoundedRect(D2D1::RectF(currentPosition.x, currentPosition.y, currentPosition.x + width, currentPosition.y + HEADER_HEIGHT), 3.0f, 3.0f), headerGeometry.ReleaseAndGetAddressOf());
				d2dFactory->CreateRoundedRectangleGeometry(D2D1::RoundedRect(D2D1::RectF(currentPosition.x, currentPosition.y + HEADER_HEIGHT, currentPosition.x + width, currentPosition.y + HEADER_HEIGHT + height), 3.0f, 3.0f), bodyGeometry.ReleaseAndGetAddressOf());

				UpdateChildrenPositions(this, Event{ EventType::WindowResize });
			}

			break;
		}
		case EventType::ChangeActiveLayer:
		{
			const auto derivedEvent = (ChangeActiveLayerEvent*)event;

			if (derivedEvent->Layer == uiLayer)
			{
				isActive = true;
			}
			else
			{
				isActive = false;
				isVisible = false;
			}
			break;
		}
		case EventType::KeyDown:
		{
			if (isActive)
			{
				const auto keyDownEvent = (KeyDownEvent*)event;

				if (keyDownEvent->CharCode == showKey)
					ToggleVisibility();
			}

			break;
		}
		case EventType::WindowResize:
		{
			const auto d2dFactory = deviceResources->GetD2DFactory();
			const auto position = GetWorldPosition();
			float startHeight = position.y;
			if (isDraggable)
			{
				d2dFactory->CreateRoundedRectangleGeometry(D2D1::RoundedRect(D2D1::RectF(position.x, position.y, position.x + width, position.y + HEADER_HEIGHT), 3.0f, 3.0f), headerGeometry.ReleaseAndGetAddressOf());
				startHeight += HEADER_HEIGHT;
			}
			d2dFactory->CreateRoundedRectangleGeometry(D2D1::RoundedRect(D2D1::RectF(position.x, startHeight, position.x + width, startHeight + height), 3.0f, 3.0f), bodyGeometry.ReleaseAndGetAddressOf());

			break;
		}
	}

	return false;
}

void UIPanel::SetChildrenAsVisible(UIComponent* uiComponent)
{
	auto children = uiComponent->GetChildren();

	for (auto i = 0; i < children.size(); i++)
	{
		auto uiComponent = (UIComponent*)children.at(i);
		if (uiComponent->followParentVisibility || uiComponent->isVisible)
			uiComponent->isVisible = !uiComponent->isVisible;

		SetChildrenAsVisible(uiComponent);
	}
}

void UIPanel::BringToFront(UIComponent* uiComponent)
{
	auto children = uiComponent->GetChildren();

	if (children.size() > 0)
		g_zIndex++;

	for (auto i = 0; i < children.size(); i++)
	{
		auto child = (UIComponent*)children.at(i);
		if (child->followParentZIndex)
			child->zIndex = g_zIndex;

		BringToFront(child);
	}
}

void UIPanel::ToggleVisibility()
{
	isVisible = !isVisible;

	SetChildrenAsVisible(this);

	if (isVisible)
	{
		g_zIndex++;
		zIndex = g_zIndex;
		BringToFront(this);
	}

	std::unique_ptr<Event> e = std::make_unique<Event>(EventType::ReorderUIComponents);
	g_eventHandler->QueueEvent(e);
}

const bool UIPanel::GetIsDragging() const { return isDragging; }

void UIPanel::UpdateChildrenPositions(UIComponent* uiComponent, const Event& e)
{
	auto children = uiComponent->GetChildren();

	for (auto i = 0; i < children.size(); i++)
	{
		auto child = (UIComponent*)children.at(i);
		child->HandleEvent(&e);

		UpdateChildrenPositions(child, e);
	}
}
