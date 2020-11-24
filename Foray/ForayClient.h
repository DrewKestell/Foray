#pragma once

#include "GameTimer.h"
#include "Game.h"
#include "Graphics/DeviceResources.h"
#include "Editor/Editor.h"
#include "Events/Observer.h"
#include "UI/UIComponent.h"
#include "UI/UILabel.h"
#include "UI/UIMenuItem.h"
#include "UI/UIMenuItemGroup.h"
#include "UI/UIPanel.h"
#include "UI/UIButton.h"
#include "UI/UIEditorToolButtonGroup.h"
#include "UI/UIEditorToolButton.h"
#include "UI/UIEditorTexturePicker.h"
#include "UI/UIInput.h"

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
	{ "Leading", DWRITE_TEXT_ALIGNMENT_LEADING },
	{ "Center", DWRITE_TEXT_ALIGNMENT_CENTER }
};

const std::unordered_map<std::string, const DWRITE_PARAGRAPH_ALIGNMENT> PARAGRAPH_ALIGNMENT_MAP
{
	{ "Near", DWRITE_PARAGRAPH_ALIGNMENT_NEAR },
	{ "Center", DWRITE_PARAGRAPH_ALIGNMENT_CENTER }
};

const std::unordered_map<std::string, Layer> LAYER_MAP
{
	{ "All",         Layer::All },
	{ "MainMenu",    Layer::MainMenu },
	{ "MainOptions", Layer::MainOptions },
	{ "Game",        Layer::Game },
	{ "Editor",      Layer::Editor }
};

class ForayClient : public IDeviceNotify, public Observer
{
private:
	GameTimer timer;
	float updateTimer{ 0.0f };
	std::unique_ptr<DeviceResources> deviceResources;
	std::vector<UIComponent*> uiComponents;
	Layer activeLayer{ Layer::MainMenu };
	std::unique_ptr<Game> game;
	std::unique_ptr<Editor> editor;

	void CreateUIElements();
	void CreateDeviceDependentResources();
	void CreateWindowSizeDependentResources();
	void InitializeTextures();
	void InitializeBrushes();
	void InitializeTextFormats();
	void InitializeLabels();
	void InitializeMenuItems();
	void InitializePanels();
	void InitializeEditorToolButtons();
	void InitializeInputs();

	// UI Json
	json brushesJson;
	json textFormatsJson;
	json labelsJson;
	json menuItemGroupsJson;
	json panelsJson;

	// Static Geometry Json
	json blocksJson;

	// UI ComPtrs
	std::unordered_map<std::string, ComPtr<ID2D1SolidColorBrush>> brushes;
	std::unordered_map<std::string, ComPtr<IDWriteTextFormat>> textFormats;

	// UI Elements
	std::unordered_map<std::string, std::unique_ptr<UILabel>> labels;
	std::unordered_map<std::string, std::unique_ptr<UIMenuItemGroup>> menuItemGroups;
	std::unordered_map<std::string, std::unique_ptr<UIMenuItem>> menuItems;
	std::unordered_map<std::string, std::unique_ptr<UIPanel>> panels;
	std::unordered_map<std::string, std::unique_ptr<UIButton>> buttons;
	std::unordered_map<std::string, std::unique_ptr<UIEditorToolButtonGroup>> editorToolButtonGroups;
	std::unordered_map<std::string, std::unique_ptr<UIEditorToolButton>> editorToolButtons;
	std::unique_ptr<UIEditorTexturePicker> editorTexturePicker;
	std::unique_ptr<UIInput> consoleInput;

	// Textures
	std::vector<ComPtr<ID3D11ShaderResourceView>> textures;

	virtual const bool HandleEvent(const Event* const event);
	void SetActiveLayer(const Layer layer);

public:
	ForayClient(const HWND window, const int width, const int height);
	void Tick();
	void OnWindowSizeChanged(const int width, const int height);
	void OnWindowMoved();
	void OnDeviceLost() override;
	void OnDeviceRestored() override;

};