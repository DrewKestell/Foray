#include "../stdafx.h"
#include "RenderingEngine.h"

RenderingEngine::RenderingEngine(DeviceResources* deviceResources)
	: deviceResources{ deviceResources }
{
}

void RenderingEngine::DrawScene()
{
	// Clear the views.
	auto d2dContext = deviceResources->GetD2DDeviceContext();
	auto d3dDevice = deviceResources->GetD3DDevice();
	auto d3dContext = deviceResources->GetD3DDeviceContext();
	auto renderTarget = deviceResources->GetOffscreenRenderTargetView();
	auto depthStencil = deviceResources->GetDepthStencilView();

	d3dContext->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
	d3dContext->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	d3dContext->OMSetRenderTargets(1, &renderTarget, depthStencil);

	// Set the viewport.
	const auto viewport = deviceResources->GetScreenViewport();
	d3dContext->RSSetViewports(1, &viewport);

	// Draw UI elements
	d2dContext->BeginDraw();

	for (auto i = 0; i < uiComponents.size(); i++)
		uiComponents.at(i)->Draw();

	// Draw Static Geometry
	if (activeLayer == Layer::Game)
	{
		for (auto& it : blocks)
			it.second->Draw();
	}

	// Debug
	//g_physicsEngine->DrawColliders(deviceResources.get(), brushes["pink"].Get());

	d2dContext->EndDraw();

	D3D11_BLEND_DESC blendStateDesc;
	ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));
	blendStateDesc.AlphaToCoverageEnable = FALSE;
	blendStateDesc.IndependentBlendEnable = FALSE;
	blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
	blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	d3dDevice->CreateBlendState(&blendStateDesc, blendState.ReleaseAndGetAddressOf());
	d3dContext->OMSetBlendState(blendState.Get(), NULL, 0xFFFFFF);

	// Draw Projectiles
	for (auto& it : projectiles)
		it->Draw(d3dContext);

	// Draw Sprites
	player.Draw(d3dContext);

	d3dContext->ResolveSubresource(deviceResources->GetBackBufferRenderTarget(), 0, deviceResources->GetOffscreenRenderTarget(), 0, DXGI_FORMAT_B8G8R8A8_UNORM);

	// Show the new frame.
	deviceResources->Present();
}