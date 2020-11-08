#pragma once

#include "SpriteVertex.h"

constexpr unsigned int SPRITE_STRIDE = sizeof(SpriteVertex);
constexpr unsigned int SPRITE_OFFSET = 0;

class Sprite
{
	const unsigned int zIndex;
	const float width;
	const float height;
	ComPtr<ID3D11InputLayout> inputLayout;
	ComPtr<ID3D11Buffer> constantBuffer;
	ComPtr<ID3D11Buffer> pixelShaderConstantBuffer;
	ComPtr<ID3D11SamplerState> samplerState;
	ComPtr<ID3D11SamplerState> mirroredSamplerState;
	ComPtr<ID3D11Buffer> vertexBuffer;
	ComPtr<ID3D11Buffer> indexBuffer;
	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;
	ID3D11ShaderResourceView* texture;

public:
	Sprite(
		ID3D11VertexShader* vertexShader,
		ID3D11PixelShader* pixelShader,
		ID3D11ShaderResourceView* texture,
		const BYTE* vertexShaderBuffer,
		const int vertexShaderSize,
		ID3D11Device* device,
		const float originX,
		const float originY,
		const float width,
		const float height,
		const unsigned int zIndex);
	void SetPosition(ID3D11Device* device, const float originX, const float originY);
	void Draw(ID3D11DeviceContext* immediateContext, const bool mirrorHorizontal);
};