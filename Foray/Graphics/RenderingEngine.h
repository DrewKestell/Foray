#pragma once

#include "DeviceResources.h"
#include "Sprite.h"
#include "../UI/UIComponent.h"
#include "../Events/Observer.h"
#include "../World/Block.h"

class RenderingEngine : public Observer
{
private:
	// constructor parameters
	DeviceResources* deviceResources;
	std::vector<UIComponent*>& uiComponents;
	std::unordered_map<std::string, std::unique_ptr<Block>>& blocks;

	// locals
	std::vector<Sprite*> sprites;
	Layer activeLayer{ Layer::MainMenu };

public:
	RenderingEngine(
		DeviceResources* deviceResources,
		std::vector<UIComponent*>& uiComponents,
		std::unordered_map<std::string, std::unique_ptr<Block>>& blocks);
	void DrawScene();
	void AddSprite(Sprite* sprite);
	void RemoveSprite(Sprite* sprite);
	~RenderingEngine();
	virtual const void HandleEvent(const Event* const event);
};