#pragma once

#include "DeviceResources.h"

class RenderingEngine
{
private:
	DeviceResources* deviceResources;

public:
	RenderingEngine(DeviceResources* deviceResources);
	void DrawScene();
};