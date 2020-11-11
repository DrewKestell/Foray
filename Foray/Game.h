#pragma once

#include "Gamepad.h"
#include "GameTimer.h"
#include "Player.h"
#include "World/Block.h"
#include "Graphics/DeviceResources.h"
#include "Graphics/Shaders/ShaderBuffer.h"
#include "Events/Observer.h"
#include "UI/UILabel.h"
#include "UI/UIMenuItem.h"
#include "UI/UIMenuItemGroup.h"

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
	float updateTimer{ 0.0f };
	std::unique_ptr<DeviceResources> deviceResources;
	std::vector<UIComponent*> uiComponents;
	Layer activeLayer{ Layer::MainMenu };
	Player* player;
	
	void PublishEvents();
	virtual const void HandleEvent(const Event* const event);
	void SetActiveLayer(const Layer layer);
	ShaderBuffer LoadShader(const std::wstring filename);

	void CreateUIElements();
	void CreateStaticGeometry();
	void CreateDeviceDependentResources();
	void CreateWindowSizeDependentResources();
	void InitializeShaders();
	void InitializeTextures();
	void InitializePlayer();
	void InitializeBrushes();
	void InitializeTextFormats();
	void InitializeLabels();
	void InitializeMenuItems();
	void InitializeBlocks();

	// Shaders
	ShaderBuffer spriteVertexShaderBuffer{};
	ComPtr<ID3D11VertexShader> spriteVertexShader;
	ShaderBuffer spritePixelShaderBuffer{};
	ComPtr<ID3D11PixelShader> spritePixelShader;

	// UI Json
	json brushesJson;
	json textFormatsJson;
	json labelsJson;
	json menuItemGroupsJson;
	
	// Static Geometry Json
	json blocksJson;

	// UI ComPtrs
	std::unordered_map<std::string, ComPtr<ID2D1SolidColorBrush>> brushes;
	std::unordered_map<std::string, ComPtr<IDWriteTextFormat>> textFormats;

	// UI Elements
	std::unordered_map<std::string, std::unique_ptr<UILabel>> labels;
	std::unordered_map<std::string, std::unique_ptr<UIMenuItemGroup>> menuItemGroups;
	std::unordered_map<std::string, std::unique_ptr<UIMenuItem>> menuItems;

	// Static Geometry
	std::unordered_map<std::string, std::unique_ptr<Block>> blocks;

	// GameObjects
	std::list<GameObject> gameObjects;

	// Textures
	std::vector<ComPtr<ID3D11ShaderResourceView>> textures;

public:
	Game();
	void Tick();
	void Initialize(const HWND window, const int width, const int height);
	void OnWindowSizeChanged(const int width, const int height);
	void OnWindowMoved();
	void OnDeviceLost() override;
	void OnDeviceRestored() override;
};