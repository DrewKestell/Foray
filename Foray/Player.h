#pragma once

#include "Sprite.h"
#include <memory>
#include "UI/Layer.h"
#include "Events/Observer.h"
#include "Events/EventHandler.h"

using namespace DirectX;

class Player : public Observer
{
private:
	// constructor params
	EventHandler& eventHandler;
	
	// device dependent resources
	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;
	const BYTE* vertexShaderBuffer;
	int vertexShaderSize;
	ID3D11Device* device;

	// locals
	XMFLOAT2 position{ 150.0f, 740.0f };
	Layer activeLayer{ Layer::MainMenu };
	bool mirrorHorizontal{ false };
	float moveAnimationTimer{ 0.0f };
	unsigned char moveFrame{ 1 };
	unsigned char lastMoveFrame{ 1 };
	ComPtr<ID3D11ShaderResourceView> moveTexture_frame1;
	ComPtr<ID3D11ShaderResourceView> moveTexture_frame2;
	ComPtr<ID3D11ShaderResourceView> moveTexture_frame3;
	ComPtr<ID3D11ShaderResourceView> moveTexture_frame4;
	ComPtr<ID3D11ShaderResourceView> moveTexture_frame5;
	std::unique_ptr<Sprite> sprite;

	// states (probably want to move this to a playerController or something
	bool movingLeft{ false };
	bool movingRight{ false };
	
public:
	Player(EventHandler& eventHandler);
	void Initialize(
		ID3D11VertexShader* vertexShader,
		ID3D11PixelShader* pixelShader,
		const BYTE* vertexShaderBuffer,
		const int vertexShaderSize,
		ID3D11Device* device);
	void Update();
	void Draw(ID3D11DeviceContext* d3dContext);
	virtual const void HandleEvent(const Event* const event);
	~Player();
};