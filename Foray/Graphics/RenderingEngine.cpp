#include "../stdafx.h"
#include "RenderingEngine.h"
#include "ConstantBufferPerObject.h"
#include "PixelShaderConstantBufferPerObject.h"
#include "../Events/ChangeActiveLayerEvent.h"
#include "../Events/EventHandler.h"

extern std::unique_ptr<EventHandler> g_eventHandler;
extern XMMATRIX g_projectionTransform;
extern XMMATRIX g_viewTransform;

RenderingEngine::RenderingEngine(
	DeviceResources* deviceResources,
	std::vector<UIComponent*>& uiComponents,
	std::vector<ComPtr<ID3D11ShaderResourceView>>& textures)
	: deviceResources{ deviceResources },
	  uiComponents{ uiComponents },
	  textures{ textures }
{
	g_eventHandler->Subscribe(*this);

	// initialize 2d sprite rendering resources
	const unsigned int indices[6]{ 0, 1, 3, 1, 2, 3 };

	// create SamplerState - TODO: move this into initialize to recover from device lost
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	auto device = deviceResources->GetD3DDevice();

	device->CreateSamplerState(&samplerDesc, samplerState.ReleaseAndGetAddressOf());

	// create constant buffer - TODO: can you pass this in as param?
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.ByteWidth = sizeof(ConstantBufferPerObject);
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;

	device->CreateBuffer(&bufferDesc, nullptr, vertexShaderConstantBuffer.ReleaseAndGetAddressOf());

	bufferDesc.ByteWidth = sizeof(PixelShaderConstantBufferPerObject);

	device->CreateBuffer(&bufferDesc, nullptr, pixelShaderConstantBuffer.ReleaseAndGetAddressOf());

	const D3D11_INPUT_ELEMENT_DESC ied[]
	{
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORDS", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	auto spriteVertexShaderBuffer = deviceResources->GetSpriteVertexShaderBuffer();
	device->CreateInputLayout(ied, ARRAYSIZE(ied), spriteVertexShaderBuffer.buffer, spriteVertexShaderBuffer.size, inputLayout.ReleaseAndGetAddressOf());

	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.ByteWidth = UINT{ sizeof(unsigned int) * 6 };
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;

	device->CreateBuffer(&bufferDesc, &indexData, indexBuffer.ReleaseAndGetAddressOf());
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

	ComPtr<ID3D11BlendState> blendState;
	d3dDevice->CreateBlendState(&blendStateDesc, blendState.ReleaseAndGetAddressOf());
	d3dContext->OMSetBlendState(blendState.Get(), NULL, 0xFFFFFF);

	// Draw Projectiles
	for (auto& it : renderComponents)
	{
		// set InputLayout
		d3dContext->IASetInputLayout(inputLayout.Get());

		// map ConstantBuffer
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		d3dContext->Map(vertexShaderConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		auto pCB = reinterpret_cast<ConstantBufferPerObject*>(mappedResource.pData);

		auto worldViewProj = g_viewTransform * g_projectionTransform;
		XMStoreFloat4x4(&pCB->gWorldViewProj, XMMatrixTranspose(worldViewProj));
		d3dContext->Unmap(vertexShaderConstantBuffer.Get(), 0);

		// setup VertexShader
		d3dContext->VSSetShader(deviceResources->GetSpriteVertexShader(), nullptr, 0);
		d3dContext->VSSetConstantBuffers(0, 1, vertexShaderConstantBuffer.GetAddressOf());

		// map ConstantBuffer
		D3D11_MAPPED_SUBRESOURCE pixelShaderMappedResource;
		d3dContext->Map(pixelShaderConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &pixelShaderMappedResource);
		auto pspCB = reinterpret_cast<PixelShaderConstantBufferPerObject*>(pixelShaderMappedResource.pData);
		pspCB->gMirrorHorizontal = it.second.MirrorHorizontal;
		d3dContext->Unmap(pixelShaderConstantBuffer.Get(), 0);

		// setup PixelShader
		d3dContext->PSSetShader(deviceResources->GetSpritePixelShader(), nullptr, 0);
		d3dContext->PSSetSamplers(0, 1, samplerState.GetAddressOf());
		d3dContext->PSSetShaderResources(0, 1, textures[it.second.TextureId].GetAddressOf());
		d3dContext->PSSetConstantBuffers(0, 1, pixelShaderConstantBuffer.GetAddressOf());

		// TODO: optimize this
		SpriteVertex vertices[4];

		// We use 1 / zIndex here to make sure sprites are correctly drawn in front of each other depending on their zIndex.
		//   zIndex of 4 should be drawn in front of zIndex of 3. vertices with lower z are drawn in front.
		//   1 / 3 = 0.3333
		//   1 / 4 = 0.25
		const auto z = 1 / static_cast<float>(it.second.ZIndex);

		const auto pos = it.second.Position;
		const auto width = it.second.Width;
		const auto height = it.second.Height;

		// top left
		SpriteVertex topLeftVertex;
		topLeftVertex.Position = XMFLOAT3{ pos.x - (width / 2), pos.y + (height / 2),  z };
		topLeftVertex.TexCoords = XMFLOAT2{ 0.0f, 0.0f };
		vertices[0] = topLeftVertex;

		// top right
		SpriteVertex topRightVertex;
		topRightVertex.Position = XMFLOAT3{ pos.x + (width / 2), pos.y + (height / 2), z };
		topRightVertex.TexCoords = XMFLOAT2{ 1.0f, 0.0f };
		vertices[1] = topRightVertex;

		// bottom right
		SpriteVertex bottomRightVertex;
		bottomRightVertex.Position = XMFLOAT3{ pos.x + (width / 2), pos.y - (height / 2), z };
		bottomRightVertex.TexCoords = XMFLOAT2{ 1.0f, 1.0f };
		vertices[2] = bottomRightVertex;

		// bottom left
		SpriteVertex bottomLeftVertex;
		bottomLeftVertex.Position = XMFLOAT3{ pos.x - (width / 2), pos.y - (height / 2), z };
		bottomLeftVertex.TexCoords = XMFLOAT2{ 0.0f, 1.0f };
		vertices[3] = bottomLeftVertex;

		D3D11_SUBRESOURCE_DATA vertexData;
		vertexData.pSysMem = vertices;

		// create vertex buffer using existing bufferDesc
		D3D11_BUFFER_DESC bufferDesc;
		ZeroMemory(&bufferDesc, sizeof(bufferDesc));
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.ByteWidth = UINT{ sizeof(SpriteVertex) * 4 };
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;

		d3dDevice->CreateBuffer(&bufferDesc, &vertexData, vertexBuffer.ReleaseAndGetAddressOf());

		// set VertexBuffer and IndexBuffer then Draw
		d3dContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &SPRITE_STRIDE, &SPRITE_OFFSET);
		d3dContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		d3dContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		d3dContext->DrawIndexed(6, 0, 0);
	}

	d3dContext->ResolveSubresource(deviceResources->GetBackBufferRenderTarget(), 0, deviceResources->GetOffscreenRenderTarget(), 0, DXGI_FORMAT_B8G8R8A8_UNORM);

	// Show the new frame.
	deviceResources->Present();
}

RenderComponent& RenderingEngine::CreateRenderComponent(
	const unsigned int gameObjectId,
	const unsigned int textureId,
	const unsigned int zIndex,
	const XMFLOAT2 position,
	const float width,
	const float height)
{
	renderComponents.insert({ gameObjectId, RenderComponent{ textureId, zIndex, position, width, height } });
	return renderComponents.at(gameObjectId);
}

void RenderingEngine::RemoveRenderComponent(const unsigned int gameObjectId)
{
	renderComponents.erase(gameObjectId);
}

const void RenderingEngine::HandleEvent(const Event* const event)
{
	const auto type = event->Type;
	switch (type)
	{
		case EventType::ChangeActiveLayer:
		{
			const auto derivedEvent = (ChangeActiveLayerEvent*)event;

			activeLayer = derivedEvent->Layer;

			break;
		}
	}
}

RenderingEngine::~RenderingEngine()
{
	g_eventHandler->Unsubscribe(*this);
}