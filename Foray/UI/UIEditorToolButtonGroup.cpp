#include "../stdafx.h"
#include "UIEditorToolButtonGroup.h"
#include "../Events/EventHandler.h"
#include "../Events/ChangeActiveLayerEvent.h"

extern std::unique_ptr<EventHandler> g_eventHandler;

UIEditorToolButtonGroup::UIEditorToolButtonGroup(const Layer uiLayer)
	: uiLayer{ uiLayer }
{
	g_eventHandler->Subscribe(*this);
}

const bool UIEditorToolButtonGroup::HandleEvent(const Event* const event)
{
	const auto type = event->Type;
	switch (type)
	{
	}

	return false;
}

void UIEditorToolButtonGroup::DeselectAllButtons()
{
	for (auto button : buttons)
	{
		button->IsSelected = false;
	}
}

const std::wstring UIEditorToolButtonGroup::GetActiveTool()
{
	for (auto button : buttons)
	{
		if (button->IsSelected)
			return button->GetText();
	}
}

UIEditorToolButtonGroup::~UIEditorToolButtonGroup()
{
	g_eventHandler->Unsubscribe(*this);
}