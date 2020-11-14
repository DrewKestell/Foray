#pragma once

class GameObject;

class BehaviorComponent
{
private:

public:
	const std::function<void(GameObject* gameObject)> OnUpdate;

	BehaviorComponent(const std::function<void(GameObject* gameObject)> onUpdate);
};