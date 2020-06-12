#pragma once

#include "Sprite.h"
#include <memory>
#include "Events/Observer.h"

class Player : public Observer
{
private:
	ComPtr<ID3D11ShaderResourceView> texture;
	std::unique_ptr<Sprite> sprite;

public:
	void Initialize(
		ID3D11VertexShader* vertexShader,
		ID3D11PixelShader* pixelShader,
		const BYTE* vertexShaderBuffer,
		const int vertexShaderSize,
		ID3D11Device* device);
	void Draw(ID3D11DeviceContext* d3dContext);
	virtual const void HandleEvent(const Event* const event);
};