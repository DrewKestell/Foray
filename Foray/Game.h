#pragma once

#include <memory>
#include "Gamepad.h"
#include "GameTimer.h"
#include "Events/Observer.h"
#include "Events/EventHandler.h"
#include "DeviceResources.h"
#include "UI/UILabel.h"
#include "UI/UIMenuItem.h"
#include "UI/UIMenuItemGroup.h"
#include "Include/nlohmann/json.hpp"

using json = nlohmann::json;

const std::unordered_map<std::string, const DWRITE_FONT_WEIGHT> FONT_WEIGHT_MAP
{
	{ "Normal", DWRITE_FONT_WEIGHT_NORMAL },
	{ "Bold",   DWRITE_FONT_WEIGHT_BOLD }
};

const std::unordered_map<std::string, const DWRITE_FONT_STYLE> FONT_STYLE_MAP
{
	{ "Normal", DWRITE_FONT_STYLE_NORMAL }
};

const std::unordered_map<std::string, const DWRITE_FONT_STRETCH> FONT_STRETCH_MAP
{
	{ "Normal", DWRITE_FONT_STRETCH_NORMAL }
};

const std::unordered_map<std::string, const DWRITE_TEXT_ALIGNMENT> TEXT_ALIGNMENT_MAP
{
	{ "Leading", DWRITE_TEXT_ALIGNMENT_LEADING }
};

const std::unordered_map<std::string, const DWRITE_PARAGRAPH_ALIGNMENT> PARAGRAPH_ALIGNMENT_MAP
{
	{ "Near", DWRITE_PARAGRAPH_ALIGNMENT_NEAR }
};

const std::unordered_map<std::string, Layer> LAYER_MAP
{
	{ "MainMenu",    Layer::MainMenu },
	{ "MainOptions", Layer::MainOptions },
	{ "Game",        Layer::Game }
};

class Game : public IDeviceNotify, public Observer
{
private:
	GameTimer timer;
	EventHandler& eventHandler;
	std::unique_ptr<DeviceResources> deviceResources;
	std::vector<UIComponent*> uiComponents;
	Layer activeLayer{ Layer::MainMenu };

	void PublishEvents();
	void Render();
	virtual const void HandleEvent(const Event* const event);
	void SetActiveLayer(const Layer layer);

	void InitializeUIElements();
	void CreateDeviceDependentResources();
	void CreateWindowSizeDependentResources();
	void InitializeBrushes();
	void InitializeTextFormats();
	void InitializeLabels();
	void InitializeMenuItems();
	
	// UI Json
	json brushesJson;
	json textFormatsJson;
	json labelsJson;
	json menuItemGroupsJson;

	// UI ComPtrs
	std::unordered_map<std::string, ComPtr<ID2D1SolidColorBrush>> brushes;
	std::unordered_map<std::string, ComPtr<IDWriteTextFormat>> textFormats;

	// UI Elements
	std::unordered_map<std::string, std::unique_ptr<UILabel>> labels;
	std::unordered_map<std::string, std::unique_ptr<UIMenuItemGroup>> menuItemGroups;
	std::unordered_map<std::string, std::unique_ptr<UIMenuItem>> menuItems;

public:
	Game(EventHandler& eventHandler);
	void Tick();
	void Initialize(const HWND window, const int width, const int height);
	void OnWindowSizeChanged(const int width, const int height);
	void OnWindowMoved();
	void OnDeviceLost() override;
	void OnDeviceRestored() override;
};