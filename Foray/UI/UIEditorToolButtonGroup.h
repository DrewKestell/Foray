#pragma once

#include "UIEditorToolButton.h"
#include "../Events/Observer.h"

class UIEditorToolButtonGroup : public Observer
{
private:
	const Layer uiLayer{ Layer::Editor };
	std::vector<UIEditorToolButton*> buttons;
public:
	UIEditorToolButtonGroup(const Layer uiLayer);
	const void HandleEvent(const Event* const event) override;
	void AddButton(UIEditorToolButton* input) { buttons.push_back(input); }
	void DeselectAllButtons();
	~UIEditorToolButtonGroup();
};
