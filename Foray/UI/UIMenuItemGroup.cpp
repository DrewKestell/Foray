#include "../stdafx.h"
#include "UIMenuItemGroup.h"
#include "../Events/EventHandler.h"
#include "../Events/ChangeActiveLayerEvent.h"
#include "../Events/GamepadInputEvent.h"
#include "../Events/KeyDownEvent.h"

extern std::unique_ptr<EventHandler> g_eventHandler;

UIMenuItemGroup::UIMenuItemGroup(const Layer uiLayer)
	: uiLayer{ uiLayer }
{
	g_eventHandler->Subscribe(*this);
}

const void UIMenuItemGroup::HandleEvent(const Event* const event)
{
	const auto type = event->Type;
	switch (type)
	{
		case EventType::ChangeActiveLayer:
		{
			const auto derivedEvent = (ChangeActiveLayerEvent*)event;

			if (derivedEvent->Layer == uiLayer)
				active = true;
			else
				active = false;

			return;
		}
		case EventType::GamepadInput:
		{
			const auto derivedEvent = (GamepadInputEvent*)event;

			if (active && derivedEvent->InputValue == XINPUT_GAMEPAD_DPAD_DOWN && derivedEvent->Pressed)
			{
				for (auto it = inputs.begin(); it != inputs.end(); it++)
				{
					if ((it + 1) != inputs.end() && (*it)->IsActive())
					{
						(*it)->SetActive(false);
						(*(it + 1))->SetActive(true);
						return;
					}
					if ((it + 1) == inputs.end())
					{
						(*it)->SetActive(false);
						(*inputs.begin())->SetActive(true);
						return;
					}
				}
			}
			else if (active && derivedEvent->InputValue == XINPUT_GAMEPAD_DPAD_UP && derivedEvent->Pressed)
			{
				for (auto it = inputs.begin(); it != inputs.end(); it++)
				{
					if (it == inputs.begin() && (*it)->IsActive())
					{
						(*it)->SetActive(false);
						(*(inputs.end() - 1))->SetActive(true);
						return;
					}
					else if ((*(it + 1))->IsActive())
					{
						(*it)->SetActive(true);
						(*(it + 1))->SetActive(false);
						return;
					}
				}
			}

			break;
		}
		case EventType::KeyDown:
		{
			const auto derivedEvent = (KeyDownEvent*)event;

			if (active && derivedEvent->CharCode == VK_DOWN)
			{
				for (auto it = inputs.begin(); it != inputs.end(); it++)
				{
					if ((it + 1) != inputs.end() && (*it)->IsActive())
					{
						(*it)->SetActive(false);
						(*(it + 1))->SetActive(true);
						return;
					}
					if ((it + 1) == inputs.end())
					{
						(*it)->SetActive(false);
						(*inputs.begin())->SetActive(true);
						return;
					}
				}
			}
			else if (active && derivedEvent->CharCode == VK_UP)
			{
				for (auto it = inputs.begin(); it != inputs.end(); it++)
				{
					if (it == inputs.begin() && (*it)->IsActive())
					{
						(*it)->SetActive(false);
						(*(inputs.end() - 1))->SetActive(true);
						return;
					}
					else if ((*(it + 1))->IsActive())
					{
						(*it)->SetActive(true);
						(*(it + 1))->SetActive(false);
						return;
					}
				}
			}

			break;
		}
	}
}

UIMenuItemGroup::~UIMenuItemGroup()
{
	g_eventHandler->Unsubscribe(*this);
}