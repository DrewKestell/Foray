#pragma once

#include "DeviceResources.h"
#include "RenderComponent.h"
#include "SpriteVertex.h"
#include "../UI/UIComponent.h"
#include "../Events/Observer.h"
#include "../World/Block.h"

static constexpr unsigned int SPRITE_STRIDE = sizeof(SpriteVertex);
static constexpr unsigned int SPRITE_OFFSET = 0;

class RenderingEngine : public Observer
{
private:
	// constructor parameters
	DeviceResources* deviceResources;
	std::vector<UIComponent*>& uiComponents;
	std::vector<ComPtr<ID3D11ShaderResourceView>>& textures;

	// locals
	Layer activeLayer{ Layer::MainMenu };
	std::unordered_map<unsigned int, RenderComponent> renderComponents;
	ComPtr<ID3D11SamplerState> samplerState;
	ComPtr<ID3D11Buffer> vertexShaderConstantBuffer;
	ComPtr<ID3D11Buffer> pixelShaderConstantBuffer;
	ComPtr<ID3D11InputLayout> inputLayout;
	ComPtr<ID3D11Buffer> indexBuffer;
	ComPtr<ID3D11Buffer> vertexBuffer;
	
public:
	RenderingEngine(
		DeviceResources* deviceResources,
		std::vector<UIComponent*>& uiComponents,
		std::vector<ComPtr<ID3D11ShaderResourceView>>& textures);
	void Initialize();
	void DrawScene();
	RenderComponent& CreateRenderComponent(
		const unsigned int gameObjectId,
		const unsigned int textureId,
		const unsigned int zIndex,
		const XMFLOAT2 position,
		const float width,
		const float height);
	void RemoveRenderComponent(const unsigned int gameObjectId);
	~RenderingEngine();
	virtual const void HandleEvent(const Event* const event);
};