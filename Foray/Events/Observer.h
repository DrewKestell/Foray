#pragma once

#include "Event.h"

class Observer
{
public:
	virtual const void HandleEvent(const Event* const event) = 0;
};