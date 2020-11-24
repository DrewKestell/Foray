#include "../stdafx.h"
#include "UIInput.h"
#include "../Events/MouseEvent.h"
#include "../Events/KeyDownEvent.h"
#include "../Events/ChangeActiveLayerEvent.h"

UIInput::UIInput(
	UIComponentArgs uiComponentArgs,
	const float width,
	const float height)
	: UIComponent(uiComponentArgs),
	  width{ width },
	  height{ height }
{
}

void UIInput::Initialize(
	ID2D1SolidColorBrush* inputBrush,
	ID2D1SolidColorBrush* inputBorderBrush,
	ID2D1SolidColorBrush* inputValueBrush,
	IDWriteTextFormat* inputValueTextFormat)
{
	this->inputBrush = inputBrush;
	this->inputBorderBrush = inputBorderBrush;
	this->inputValueBrush = inputValueBrush;
	this->inputValueTextFormat = inputValueTextFormat;

	const auto position = GetWorldPosition();
	deviceResources->GetD2DFactory()->CreateRectangleGeometry(D2D1_RECT_F{ position.x, position.y, position.x + width, position.y + height }, inputGeometry.ReleaseAndGetAddressOf());
}

void UIInput::Draw()
{
	if (!isVisible) return;

	// Draw Input
	const auto position = GetWorldPosition();
	auto d2dDeviceContext = deviceResources->GetD2DDeviceContext();
	d2dDeviceContext->FillGeometry(inputGeometry.Get(), inputBrush);
	d2dDeviceContext->DrawGeometry(inputGeometry.Get(), inputBorderBrush, 2.0f);

	// Draw Input Text   
	if (inputIndex > 0)
		d2dDeviceContext->DrawTextLayout(D2D1::Point2F(position.x + 6.0f, position.y + 1.0f), inputValueTextLayout.Get(), inputValueBrush);
}

const bool UIInput::HandleEvent(const Event* const event)
{
	// first pass the event to UIComponent base so it can reset localPosition based on new client dimensions
	UIComponent::HandleEvent(event);

	const auto type = event->Type;
	switch (type)
	{
		case EventType::LeftMouseDown:
		{
			const auto wasActive = active;
			active = false;

			const auto mouseDownEvent = (MouseEvent*)event;

			if (isVisible)
			{
				const auto position = GetWorldPosition();
				if (Utility::DetectClick(position.x, position.y, position.x + width, position.y + height, mouseDownEvent->MousePosX, mouseDownEvent->MousePosY))
					active = true;

				if (wasActive != active)
					CreateTextLayout();
			}

			break;
		}
		case EventType::KeyDown:
		{
			if (active)
			{
				const auto keyDownEvent = (KeyDownEvent*)event;

				if (inputIndex <= 50)
				{
					inputValue[inputIndex] = keyDownEvent->CharCode;
					inputIndex++;

					CreateTextLayout();
				}
			}

			break;
		}
		case EventType::SystemKeyDown:
		{
			if (active)
			{
				const auto keyDownEvent = (KeyPressEvent*)event;
				const auto charCode = keyDownEvent->CharCode;

				switch (charCode)
				{
				case VK_BACK:
				{
					if (inputIndex > 0)
					{
						inputValue[inputIndex - 1] = 0;
						inputIndex--;

						CreateTextLayout();
					}
					break;
				}
				}
			}

			break;
		}
		case EventType::ChangeActiveLayer:
		{
			const auto derivedEvent = (ChangeActiveLayerEvent*)event;

			if (derivedEvent->Layer == uiLayer || uiLayer == Layer::All)
				isVisible = true;
			else
				isVisible = false;

			break;
		}
		case EventType::WindowResize:
		{
			const auto position = GetWorldPosition();
			deviceResources->GetD2DFactory()->CreateRectangleGeometry(D2D1_RECT_F{ position.x, position.y, position.x + width, position.y + height }, inputGeometry.ReleaseAndGetAddressOf());
		}
	}

	return false;
}

const wchar_t* UIInput::GetInputValue() const
{
	return inputValue;
}

void UIInput::ClearInput()
{
	active = false;

	inputIndex = 0;
	ZeroMemory(inputValue, sizeof(inputValue));
}

void UIInput::CreateTextLayout()
{
	std::wostringstream outInputValue;
	outInputValue << inputValue;
	if (active)
		outInputValue << "|";

	deviceResources->GetWriteFactory()->CreateTextLayout(
		outInputValue.str().c_str(),
		(UINT32)outInputValue.str().size(),
		inputValueTextFormat,
		width,
		height - 2, // (height - 2) takes the border into account, and looks more natural
		inputValueTextLayout.ReleaseAndGetAddressOf()
	);
}