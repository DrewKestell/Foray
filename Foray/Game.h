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

	void CreateLabels();
	void CreateMenuItems();

	void CreateDeviceDependentResources();
	void CreateWindowSizeDependentResources();
	void InitializeBrushes();
	void InitializeTextFormats();
	void InitializeLabels();
	void InitializeMenuItems();

	// Brushes
	ComPtr<ID2D1SolidColorBrush> whiteBrush;

	// TextFormats
	ComPtr<IDWriteTextFormat> gameTitleTextFormat;
	ComPtr<IDWriteTextFormat> defaultTextFormat;
	ComPtr<IDWriteTextFormat> bulletTextFormat;

	// Labels
	std::unique_ptr<UILabel> menuScreen_gameTitleLabel;
	std::unique_ptr<UILabel> menuScreen_mainOptionsLabel;

	// MenuItems
	std::unique_ptr<UIMenuItem> menuScreen_startMenuItem;
	std::unique_ptr<UIMenuItem> menuScreen_optionsMenuItem;
	std::unique_ptr<UIMenuItem> menuScreen_mainOptionsBackItem;

	// MenuItemGroups
	std::unique_ptr<UIMenuItemGroup> menuScreen_menuItemGroup;
	std::unique_ptr<UIMenuItemGroup> menuScreen_mainOptionsGroup;

public:
	Game(EventHandler& eventHandler);
	void Initialize(HWND window, int width, int height);
	void Tick();
	void OnWindowSizeChanged(int width, int height);
	void OnWindowMoved();

	// IDeviceNotify
	void OnDeviceLost() override;
	void OnDeviceRestored() override;
};