#include "stdafx.h"
#include <queue>
#include "Game.h"
#include "Events/Event.h"
#include "Events/Observer.h"
#include <DirectXColors.h>

Game::Game(EventHandler& eventHandler)
	: eventHandler{ eventHandler }
{
	eventHandler.Subscribe(*this);

	deviceResources = std::make_unique<DeviceResources>();
	deviceResources->RegisterDeviceNotify(this);
}

void Game::Initialize(HWND window, int width, int height)
{
	deviceResources->SetWindow(window, width, height);
	deviceResources->CreateDeviceResources();
	CreateDeviceDependentResources();

	deviceResources->CreateWindowSizeDependentResources();
	CreateWindowSizeDependentResources();

	timer.Reset();
}

void Game::CreateDeviceDependentResources()
{
	// TODO
}

void Game::CreateWindowSizeDependentResources()
{
	// TODO
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

	const auto d3dContext = deviceResources->GetD3DDeviceContext();
	d3dContext->ResolveSubresource(deviceResources->GetBackBufferRenderTarget(), 0, deviceResources->GetOffscreenRenderTarget(), 0, DXGI_FORMAT_B8G8R8A8_UNORM);

	// Show the new frame.
	deviceResources->Present();
}

const void Game::HandleEvent(const Event* const event)
{
	// TODO
}

void Game::OnWindowSizeChanged(int width, int height)
{
	// TODO
}

void Game::OnWindowMoved()
{
	// TODO
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
	// TODO
}