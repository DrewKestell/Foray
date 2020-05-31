#pragma once

#include <memory>
#include "Gamepad.h"
#include "GameTimer.h"
#include "Events/Observer.h"
#include "Events/EventHandler.h"
#include "DeviceResources.h"

class Game : public IDeviceNotify, public Observer
{
private:
	GameTimer timer;
	EventHandler& eventHandler;
	std::unique_ptr<DeviceResources> deviceResources;
	void PublishEvents();
	void Render();
	virtual const void HandleEvent(const Event* const event);
public:
	Game(EventHandler& eventHandler);
	void Initialize(HWND window, int width, int height);
	void CreateDeviceDependentResources();
	void CreateWindowSizeDependentResources();
	void Tick();
	void OnWindowSizeChanged(int width, int height);
	void OnWindowMoved();

	// IDeviceNotify
	void OnDeviceLost() override;
	void OnDeviceRestored() override;
};