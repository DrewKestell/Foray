#include "../stdafx.h"
#include "UIMenuItemGroup.h"
#include "../Events/ChangeActiveLayerEvent.h"
#include "../Utility.h"

UIMenuItemGroup::UIMenuItemGroup(
	UIComponentArgs uiComponentArgs)
	: UIComponent(uiComponentArgs)
{
}

void UIMenuItemGroup::Initialize()
{
}

void UIMenuItemGroup::Draw()
{
	if (!isVisible) return;
}

const void UIMenuItemGroup::HandleEvent(const Event* const event)
{
	// first pass the event to UIComponent base so it can reset localPosition based on new client dimensions
	UIComponent::HandleEvent(event);

	const auto type = event->type;
	switch (type)
	{
	case EventType::ChangeActiveLayer:
	{
		const auto derivedEvent = (ChangeActiveLayerEvent*)event;

		if (derivedEvent->layer == uiLayer && GetParent() == nullptr)
			isVisible = true;
		else
			isVisible = false;

		break;
	}
	}
}