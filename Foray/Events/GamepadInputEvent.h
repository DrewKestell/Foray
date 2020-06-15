#pragma once

#include "Event.h"

class GamepadInputEvent : public Event
{
public:
	GamepadInputEvent(const EventType eventType, const unsigned short inputValue, const short modifier, const bool pressed)
		: Event(eventType),
		  inputValue{ inputValue },
		  modifier{ modifier },
		  pressed{ pressed }
	{
	}

	// will be one of the gamepad constants defined in Xinput.h, eg: XINPUT_GAMEPAD_DPAD_UP
	const unsigned short inputValue; 

	// thumbstick and trigger input will have an additional modifier value set. 0 otherwise.
	// more info here: https://docs.microsoft.com/en-us/windows/win32/xinput/getting-started-with-xinput
	const short modifier;

	// true when pressed
	const bool pressed;
};