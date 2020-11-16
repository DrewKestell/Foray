#pragma once

#include "../UI/Layer.h"
#include "../Events/Observer.h"

class Editor : public Observer
{
private:
	Layer activeLayer{ Layer::MainMenu };

	virtual const void HandleEvent(const Event* const event);

public:

	Editor();
	void Initialize();
	void Tick();
	void Initialize(const HWND window, const int width, const int height);
};