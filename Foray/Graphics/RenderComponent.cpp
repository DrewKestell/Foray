#include "../stdafx.h"
#include "RenderComponent.h"

RenderComponent::RenderComponent(
	const unsigned int textureId,
	const unsigned int zIndex,
	const XMFLOAT2 position,
	const float width,
	const float height,
	const bool convertToWorldPos)
	: TextureId{ textureId },
	  ZIndex{ zIndex },
	  Width{ width },
	  Height{ height }
{
	if (convertToWorldPos)
		SetPosition(position);
	else
		Position = position;
}

void RenderComponent::SetPosition(const XMFLOAT2 newPos)
{
	Position = Utility::ConvertToWorldSpace(newPos);
}