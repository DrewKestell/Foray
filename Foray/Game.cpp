#include "stdafx.h"
#include <queue>
#include <DirectXColors.h>
#include "Game.h"
#include "Constants.h"
#include "Events/Event.h"
#include "Events/Observer.h"
#include "Events/ChangeActiveLayerEvent.h"
#include "Events/GamepadInputEvent.h"

float g_clientWidth{ CLIENT_WIDTH };
float g_clientHeight{ CLIENT_HEIGHT };

Game::Game(EventHandler& eventHandler)
	: eventHandler{ eventHandler }
{
	eventHandler.Subscribe(*this);

	deviceResources = std::make_unique<DeviceResources>();
	deviceResources->RegisterDeviceNotify(this);
}

void Game::Initialize(HWND window, int width, int height)
{
	CreateLabels();
	CreateMenuItems();

	deviceResources->SetWindow(window, width, height);
	deviceResources->CreateDeviceResources();
	CreateDeviceDependentResources();

	deviceResources->CreateWindowSizeDependentResources();
	CreateWindowSizeDependentResources();

	timer.Reset();
	SetActiveLayer(Layer::MainMenu);
}

void Game::CreateLabels()
{
	menuScreen_gameTitleLabel = std::make_unique<UILabel>(UIComponentArgs{ deviceResources.get(), uiComponents, [](const float, const float) { return XMFLOAT2{ 20.0f, 10.0f }; }, Layer::MainMenu, 0 }, 100.0f);
	menuScreen_mainOptionsLabel = std::make_unique<UILabel>(UIComponentArgs{ deviceResources.get(), uiComponents, [](const float, const float) { return XMFLOAT2{ 20.0f, 10.0f }; }, Layer::MainOptions, 0 }, 400.0f);
}

void Game::CreateMenuItems()
{
	const auto onActivateStartItem = [this]()
	{
		SetActiveLayer(Layer::Game);
	};
	menuScreen_startMenuItem = std::make_unique<UIMenuItem>(UIComponentArgs{ deviceResources.get(), uiComponents, [](const float, const float) { return XMFLOAT2{ 42.0f, 100.0f }; }, Layer::MainMenu, 0 }, 100.0f, "Start", onActivateStartItem, true);
	
	const auto onActivateOptionsItem = [this]()
	{
		SetActiveLayer(Layer::MainOptions);
	};
	menuScreen_optionsMenuItem = std::make_unique<UIMenuItem>(UIComponentArgs{ deviceResources.get(), uiComponents, [](const float, const float) { return XMFLOAT2{ 42.0f, 120.0f }; }, Layer::MainMenu, 0 }, 100.0f, "Options", onActivateOptionsItem);
	
	menuScreen_menuItemGroup = std::make_unique<UIMenuItemGroup>(Layer::MainMenu, eventHandler);
	menuScreen_menuItemGroup->AddInput(menuScreen_startMenuItem.get());
	menuScreen_menuItemGroup->AddInput(menuScreen_optionsMenuItem.get());

	const auto onActivateMainOptionsBackItem = [this]()
	{
		SetActiveLayer(Layer::MainMenu);
	};
	menuScreen_mainOptionsBackItem = std::make_unique<UIMenuItem>(UIComponentArgs{ deviceResources.get(), uiComponents, [](const float, const float) { return XMFLOAT2{ 42.0f, 100.0f }; }, Layer::MainOptions, 0 }, 100.0f, "Back", onActivateMainOptionsBackItem);

	menuScreen_mainOptionsGroup = std::make_unique<UIMenuItemGroup>(Layer::MainMenu, eventHandler);
	menuScreen_mainOptionsGroup->AddInput(menuScreen_mainOptionsBackItem.get());
}

void Game::CreateDeviceDependentResources()
{
	InitializeBrushes();
	InitializeTextFormats();
	InitializeLabels();
	InitializeMenuItems();
}

void Game::CreateWindowSizeDependentResources()
{
	// TODO
}

void Game::InitializeBrushes()
{
	auto d2dContext = deviceResources->GetD2DDeviceContext();

	d2dContext->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f), whiteBrush.ReleaseAndGetAddressOf());
}

void Game::InitializeTextFormats()
{
	auto writeFactory = deviceResources->GetWriteFactory();

	writeFactory->CreateTextFormat(TREBUCHET_FONT_FAMILY, nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 40.0f, LOCALE, gameTitleTextFormat.ReleaseAndGetAddressOf());
	gameTitleTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	gameTitleTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

	writeFactory->CreateTextFormat(ARIAL_FONT_FAMILY, nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 14.0f, LOCALE, defaultTextFormat.ReleaseAndGetAddressOf());
	defaultTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	defaultTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

	writeFactory->CreateTextFormat(ARIAL_FONT_FAMILY, nullptr, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 15.0f, LOCALE, bulletTextFormat.ReleaseAndGetAddressOf());
	bulletTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	bulletTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
}

void Game::InitializeLabels()
{
	menuScreen_gameTitleLabel->Initialize(whiteBrush.Get(), gameTitleTextFormat.Get());
	menuScreen_gameTitleLabel->SetText(L"Foray");

	menuScreen_mainOptionsLabel->Initialize(whiteBrush.Get(), gameTitleTextFormat.Get());
	menuScreen_mainOptionsLabel->SetText(L"Foray ⇾ Options");
}

void Game::InitializeMenuItems()
{
	menuScreen_startMenuItem->Initialize(whiteBrush.Get(), defaultTextFormat.Get(), bulletTextFormat.Get());
	menuScreen_optionsMenuItem->Initialize(whiteBrush.Get(), defaultTextFormat.Get(), bulletTextFormat.Get());
	menuScreen_mainOptionsBackItem->Initialize(whiteBrush.Get(), defaultTextFormat.Get(), bulletTextFormat.Get());

}

void Game::Tick()
{
	timer.Tick();

	PublishEvents();

	Render();
}

void Game::PublishEvents()
{
	std::queue<std::unique_ptr<const Event>>& eventQueue = eventHandler.GetEventQueue();
	while (!eventQueue.empty())
	{
		auto event = std::move(eventQueue.front());
		eventQueue.pop();

		// first let the ui elements handle the event
		for (auto i = (int)uiComponents.size() - 1; i >= 0; i--)
		{
			uiComponents.at(i)->HandleEvent(event.get());
		}

		// next let game objects handle the event
		std::list<Observer*>& observers = eventHandler.GetObservers();
		for (auto observer : observers)
		{
			observer->HandleEvent(event.get());
		}
	}
}

void Game::Render()
{
	// Clear the views.
	auto context = deviceResources->GetD3DDeviceContext();
	auto renderTarget = deviceResources->GetOffscreenRenderTargetView();
	auto depthStencil = deviceResources->GetDepthStencilView();

	context->ClearRenderTargetView(renderTarget, DirectX::Colors::CornflowerBlue);
	context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	context->OMSetRenderTargets(1, &renderTarget, depthStencil);

	// Set the viewport.
	const auto viewport = deviceResources->GetScreenViewport();
	context->RSSetViewports(1, &viewport);

	// Draw UI elements
	auto d2dContext = deviceResources->GetD2DDeviceContext();

	d2dContext->BeginDraw();

	for (auto i = 0; i < uiComponents.size(); i++)
		uiComponents.at(i)->Draw();

	d2dContext->EndDraw();

	const auto d3dContext = deviceResources->GetD3DDeviceContext();
	d3dContext->ResolveSubresource(deviceResources->GetBackBufferRenderTarget(), 0, deviceResources->GetOffscreenRenderTarget(), 0, DXGI_FORMAT_B8G8R8A8_UNORM);

	// Show the new frame.
	deviceResources->Present();
}

const void Game::HandleEvent(const Event* const event)
{
	const auto type = event->type;
	switch (type)
	{
		case EventType::GamepadInput:
		{
			const auto derivedEvent = (GamepadInputEvent*)event;

			if (derivedEvent->inputValue == XINPUT_GAMEPAD_B)
			{
				if (activeLayer == Layer::MainOptions)
					SetActiveLayer(Layer::MainMenu);
			}
		}
	}
}

void Game::SetActiveLayer(const Layer layer)
{
	activeLayer = layer;

	std::unique_ptr<Event> e = std::make_unique<ChangeActiveLayerEvent>(layer);
	eventHandler.QueueEvent(e);
}

void Game::OnWindowSizeChanged(int width, int height)
{
	if (!deviceResources->WindowSizeChanged(width, height))
		return;

	g_clientWidth = static_cast<float>(width);
	g_clientHeight = static_cast<float>(height);

	CreateWindowSizeDependentResources();
}

void Game::OnWindowMoved()
{
	const auto r = deviceResources->GetOutputSize();
	deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnDeviceLost()
{
	// TODO: Add Direct3D resource cleanup here.
	// may not be needed - classes should be designed with RAII in mind, and clean up
	// after themselves. not sure if there is an exception to this with D3D,
	// so i'm leaving this stubbed out for now.
}

void Game::OnDeviceRestored()
{
	CreateDeviceDependentResources();

	CreateWindowSizeDependentResources();

	SetActiveLayer(activeLayer);
}
