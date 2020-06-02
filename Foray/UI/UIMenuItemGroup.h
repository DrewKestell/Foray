#pragma once

#include "UIMenuItem.h"
#include "../Events/EventHandler.h"
#include "../Events/Observer.h"

class UIMenuItemGroup : public Observer
{
	const Layer uiLayer{ Layer::MainMenu };
	EventHandler& eventHandler;
	bool active{ false };
	std::vector<UIMenuItem*> inputs;
public:
	UIMenuItemGroup(const Layer uiLayer, EventHandler& eventHandler);
	const void HandleEvent(const Event* const event) override;
	void AddInput(UIMenuItem* input) { inputs.push_back(input); }
	~UIMenuItemGroup();
};