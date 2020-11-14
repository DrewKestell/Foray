#pragma once

#include "BehaviorComponent.h"
#include "Physics/Collider.h"
#include "Graphics/RenderComponent.h"

static unsigned int IdCounter;

class GameObject
{
private:

public:
	bool operator == (const GameObject& go) const { return GameObjectId == go.GameObjectId; }
	bool operator != (const GameObject& go) const { return !operator==(go); }

	const unsigned int GameObjectId{ IdCounter++ };
	XMFLOAT2 Position{ 0.0f, 0.0f };
	std::vector<BehaviorComponent> BehaviorComponents;
	RenderComponent* RenderComponent;
	Collider* Collider;

	void Translate(const XMFLOAT2 vector);
};