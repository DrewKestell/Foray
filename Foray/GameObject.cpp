#include "stdafx.h"
#include "GameObject.h"

void GameObject::Translate(const XMFLOAT2 vector)
{
	Position += vector;
	
	if (Collider)
	{
		Collider->Position += vector;
	}

	if (RenderComponent)
	{
		RenderComponent->SetPosition(Position);
	}
}