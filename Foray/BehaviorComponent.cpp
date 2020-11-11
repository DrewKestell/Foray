#include "stdafx.h"
#include "BehaviorComponent.h"

BehaviorComponent::BehaviorComponent(const std::function<void(GameObject* gameObject)> onUpdate)
	: OnUpdate{ onUpdate }
{
}