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

const void UIEditorToolButtonGroup::HandleEvent(const Event* const event)
{
	const auto type = event->Type;
	switch (type)
	{
	}
}

void UIEditorToolButtonGroup::DeselectAllButtons()
{
	for (auto button : buttons)
	{
		button->IsSelected = false;
	}
}

UIEditorToolButtonGroup::~UIEditorToolButtonGroup()
{
	g_eventHandler->Unsubscribe(*this);
}