#include "../stdafx.h"
#include <Windows.h>
#include <Xinput.h>
#include "UIMenuItemGroup.h"
#include "../Events/ChangeActiveLayerEvent.h"
#include "../Events/GamepadInputEvent.h"

UIMenuItemGroup::UIMenuItemGroup(const Layer uiLayer, EventHandler& eventHandler)
	: uiLayer{ uiLayer },
	  eventHandler{ eventHandler }
{
	eventHandler.Subscribe(*this);
}

const void UIMenuItemGroup::HandleEvent(const Event* const event)
{
	const auto type = event->type;
	switch (type)
	{
		case EventType::ChangeActiveLayer:
		{
			const auto derivedEvent = (ChangeActiveLayerEvent*)event;

			if (derivedEvent->layer == uiLayer)
				active = true;
			else
				active = false;

			return;
		}
		case EventType::GamepadInput:
		{
			const auto derivedEvent = (GamepadInputEvent*)event;

			if (active && derivedEvent->inputValue == XINPUT_GAMEPAD_DPAD_DOWN)
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
			else if (active && derivedEvent->inputValue == XINPUT_GAMEPAD_DPAD_UP)
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
		}
	}
}

UIMenuItemGroup::~UIMenuItemGroup()
{
	eventHandler.Unsubscribe(*this);
}