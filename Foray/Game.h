#pragma once

#include "Player.h"
#include "Events/Observer.h"

class Game : public Observer
{
private:
	Layer activeLayer{ Layer::MainMenu };

	Player* player{ nullptr };

	// Static Geometry Json
	json blocksJson;
	
	virtual const bool HandleEvent(const Event* const event);
	void CreateStaticGeometry();
	void CreatePlayer();

public:
	Game();
	void Initialize();
	void Tick();

};