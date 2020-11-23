#pragma once

#include "../UI/Layer.h"
#include "../UI/UIEditorToolButtonGroup.h"
#include "../UI/UIEditorTexturePicker.h"
#include "../Events/Observer.h"

class Editor : public Observer
{
private:
	Layer activeLayer{ Layer::MainMenu };
	std::unordered_map<int, std::unordered_map<int, int>> mapTiles;
	virtual const bool HandleEvent(const Event* const event);
	UIEditorToolButtonGroup* toolButtonGroup;
	UIEditorTexturePicker* texturePicker;

public:

	Editor(UIEditorToolButtonGroup* toolButtonGroup, UIEditorTexturePicker* texturePicker);
	void Initialize();
	void Tick();
	void Initialize(const HWND window, const int width, const int height);
	void ClearTiles();
	void CreateTile(const int x, const int y);
	void RemoveTile(const int x, const int y);
};