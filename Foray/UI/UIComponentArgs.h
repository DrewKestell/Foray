#pragma once

#include <vector>
#include <functional>
#include <DirectXMath.h>
#include "Layer.h"
#include "../DeviceResources.h"

class UIComponent;

struct UIComponentArgs
{
	UIComponentArgs(
		DeviceResources* deviceResources,
		std::vector<UIComponent*>& uiComponents,
		const std::function<DirectX::XMFLOAT2(const float width, const float height)> calculatePosition,
		const Layer uiLayer,
		const unsigned int zIndex)
		: deviceResources{ deviceResources },
		  uiComponents{ uiComponents },
		  calculatePosition{ calculatePosition },
		  uiLayer{ uiLayer },
		  zIndex{ zIndex }
	{
	}

	DeviceResources* deviceResources;
	std::vector<UIComponent*>& uiComponents;
	const std::function<DirectX::XMFLOAT2(const float width, const float height)> calculatePosition;
	const Layer uiLayer;
	const unsigned int zIndex;
};