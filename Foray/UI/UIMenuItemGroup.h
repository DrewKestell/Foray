#pragma once

#include "UIComponent.h"
#include "../Events/Observer.h"
#include "../Events/Event.h"

class UIMenuItemGroup : public UIComponent
{
public:
	UIMenuItemGroup(UIComponentArgs uiComponentArgs);
	void Initialize();
	void Draw() override;
	const void HandleEvent(const Event* const event) override;
};