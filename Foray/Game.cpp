#include "stdafx.h"
#include <queue>
#include <DirectXColors.h>
#include <fstream>
#include <iostream>
#include "Game.h"
#include "Constants.h"
#include "Utility.h"
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

void Game::Initialize(const HWND window, const int width, const int height)
{
	deviceResources->SetWindow(window, width, height);

	InitializeUIElements();

	deviceResources->CreateDeviceResources();
	CreateDeviceDependentResources();

	deviceResources->CreateWindowSizeDependentResources();
	CreateWindowSizeDependentResources();

	timer.Reset();
	SetActiveLayer(Layer::MainMenu);
}

void Game::InitializeUIElements()
{
	wchar_t buffer[MAX_PATH];
	GetCurrentDirectory(sizeof(buffer), buffer);

	std::ifstream i;
	i.open("./UI/ui-elements.json");

	json j;
	i >> j;

	const auto uiElements = j["uiElements"];

	brushesJson = uiElements["brushes"];
	for (auto i = 0; i < brushesJson.size(); i++)
	{
		const auto brush = brushesJson[i];
		const auto id = brush["id"].get<std::string>();
		brushes[id] = ComPtr<ID2D1SolidColorBrush>();
	}

	textFormatsJson = uiElements["textFormats"];
	for (auto i = 0; i < textFormatsJson.size(); i++)
	{
		const auto textFormat = textFormatsJson[i];
		const auto id = textFormat["id"].get<std::string>();
		textFormats[id] = ComPtr<IDWriteTextFormat>();
	}

	labelsJson = uiElements["labels"];
	for (auto i = 0; i < labelsJson.size(); i++)
	{
		const auto label = labelsJson[i];
		const auto id = label["id"].get<std::string>();
		const auto x = label["x"].get<float>();
		const auto y = label["y"].get<float>();
		const auto layer = LAYER_MAP.at(label["layer"].get<std::string>());
		const auto zIndex = label["zIndex"].get<unsigned int>();
		labels[id] = std::make_unique<UILabel>(UIComponentArgs{ deviceResources.get(), uiComponents, [x, y](const float, const float) { return XMFLOAT2{ x, y }; }, layer, zIndex });
	}

	menuItemGroupsJson = uiElements["menuItemGroups"];
	for (auto i = 0; i < menuItemGroupsJson.size(); i++)
	{
		const auto menuItemGroupJson = menuItemGroupsJson[i];
		const auto menuItemGroupId = menuItemGroupJson["id"].get<std::string>();
		
		menuItemGroups[menuItemGroupId] = std::make_unique<UIMenuItemGroup>(Layer::MainMenu, eventHandler);

		const auto menuItemsJson = menuItemGroupJson["menuItems"];

		for (auto j = 0; j < menuItemsJson.size(); j++)
		{
			const auto menuItem = menuItemsJson[j];
			const auto menuItemId = menuItem["id"].get<std::string>();
			const auto x = menuItem["x"].get<float>();
			const auto y = menuItem["y"].get<float>();
			const auto layer = LAYER_MAP.at(menuItem["layer"].get<std::string>());
			const auto zIndex = menuItem["zIndex"].get<unsigned int>();
			const auto text = menuItem["text"].get<std::string>();
			const auto isActive = menuItem["isActive"].get<bool>();

			std::function<void()> onActivate;
			if (menuItemId == "menuScreen_start")
			{
				onActivate = [this]()
				{
					SetActiveLayer(Layer::Game);
				};
			}
			else if (menuItemId == "menuScreen_options")
			{
				onActivate = [this]()
				{
					SetActiveLayer(Layer::MainOptions);
				};
			}
			else if (menuItemId == "menuScreen_mainOptionsBack")
			{
				onActivate = [this]()
				{
					SetActiveLayer(Layer::MainMenu);
				};
			}

			menuItems[menuItemId] = std::make_unique<UIMenuItem>(UIComponentArgs{ deviceResources.get(), uiComponents, [x, y](const float, const float) { return XMFLOAT2{ x, y }; }, layer, zIndex }, text.c_str(), onActivate, isActive);
			menuItemGroups[menuItemGroupId]->AddInput(menuItems[menuItemId].get());
		}
	}
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

	for (auto i = 0; i < brushesJson.size(); i++)
	{
		const auto brush = brushesJson[i];
		const auto id = brush["id"].get<std::string>();
		const auto r = brush["r"].get<float>();
		const auto g = brush["g"].get<float>();
		const auto b = brush["b"].get<float>();
		const auto a = brush["a"].get<float>();

		d2dContext->CreateSolidColorBrush(D2D1::ColorF(r, g, b, a), brushes[id].ReleaseAndGetAddressOf());
	}
}

void Game::InitializeTextFormats()
{
	auto writeFactory = deviceResources->GetWriteFactory();

	for (auto i = 0; i < textFormatsJson.size(); i++)
	{
		const auto textFormat = textFormatsJson[i];
		const auto id = textFormat["id"].get<std::string>();
		const auto fontFamily = Utility::s2ws(textFormat["fontFamily"].get<std::string>());
		const auto weight = FONT_WEIGHT_MAP.at(textFormat["weight"].get<std::string>());
		const auto style = FONT_STYLE_MAP.at(textFormat["style"].get<std::string>());
		const auto stretch = FONT_STRETCH_MAP.at(textFormat["stretch"].get<std::string>());
		const auto size = textFormat["size"].get<float>();
		const auto textAlignment = TEXT_ALIGNMENT_MAP.at(textFormat["textAlignment"].get<std::string>());
		const auto paragraphAlignment = PARAGRAPH_ALIGNMENT_MAP.at(textFormat["paragraphAlignment"].get<std::string>());

		writeFactory->CreateTextFormat(fontFamily.c_str(), nullptr, weight, style, stretch, size, LOCALE, textFormats.at(id).ReleaseAndGetAddressOf());
		textFormats.at(id)->SetTextAlignment(textAlignment);
		textFormats.at(id)->SetParagraphAlignment(paragraphAlignment);
	}
}

void Game::InitializeLabels()
{
	for (auto i = 0; i < labelsJson.size(); i++)
	{
		const auto labelJson = labelsJson[i];
		const auto id = labelJson["id"].get<std::string>();
		const auto label = labels.at(id).get();
		const auto brushId = labelJson["brushId"].get<std::string>();
		const auto textFormatId = labelJson["textFormatId"].get<std::string>();
		const auto initialText = Utility::s2ws(labelJson["initialText"].get<std::string>());
		label->Initialize(brushes.at(brushId).Get(), textFormats.at(textFormatId).Get());
		label->SetText(initialText.c_str());
	}
}

void Game::InitializeMenuItems()
{
	for (auto i = 0; i < menuItemGroupsJson.size(); i++)
	{
		const auto menuItemsJson = menuItemGroupsJson[i]["menuItems"];

		for (auto j = 0; j < menuItemsJson.size(); j++)
		{
			const auto menuItem = menuItemsJson[j];
			const auto id = menuItem["id"].get<std::string>();
			const auto brushId = menuItem["brushId"].get<std::string>();
			const auto bodyTextFormatId = menuItem["bodyTextFormatId"].get<std::string>();
			const auto bulletTextFormatId = menuItem["bulletTextFormatId"].get<std::string>();

			menuItems.at(id)->Initialize(brushes.at(brushId).Get(), textFormats.at(bodyTextFormatId).Get(), textFormats.at(bulletTextFormatId).Get());
		}
	}
}

void Game::OnWindowSizeChanged(const int width, const int height)
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
