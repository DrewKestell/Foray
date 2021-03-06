#pragma once

#include "UIMenuItem.h"
#include "../Events/Observer.h"

class UIMenuItemGroup : public Observer
{
	const Layer uiLayer{ Layer::MainMenu };
	bool active{ false };
	std::vector<UIMenuItem*> inputs;
public:
	UIMenuItemGroup(const Layer uiLayer);
	const bool HandleEvent(const Event* const event) override;
	void AddInput(UIMenuItem* input) { inputs.push_back(input); }
	~UIMenuItemGroup();
};