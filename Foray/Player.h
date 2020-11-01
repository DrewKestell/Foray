#pragma once

#include "Sprite.h"
#include <memory>
#include "Physics/Collider.h"
#include "Physics/CollisionResult.h"
#include "UI/Layer.h"
#include "Events/Observer.h"
#include "Events/EventHandler.h"
#include "GameObject.h"

using namespace DirectX;

class Player : public Observer, public GameObject
{
private:
	// constructor params
	EventHandler& eventHandler;

	// device dependent resources
	ID3D11VertexShader* vertexShader{ nullptr };
	ID3D11PixelShader* pixelShader{ nullptr };
	const BYTE* vertexShaderBuffer{ 0 };
	int vertexShaderSize{ 0 };
	ID3D11Device* device{ nullptr };

	// locals
	XMFLOAT2 position{ 150.0f, 200.0f };
	Layer activeLayer{ Layer::MainMenu };
	bool mirrorHorizontal{ false };
	float moveAnimationTimer{ 0.0f };
	float shootAnimationTimer{ 0.0f };
	unsigned char moveFrame{ 1 };
	unsigned char lastMoveFrame{ 1 };
	ComPtr<ID3D11ShaderResourceView> moveTexture_frame1;
	ComPtr<ID3D11ShaderResourceView> moveTexture_frame2;
	ComPtr<ID3D11ShaderResourceView> moveTexture_frame3;
	ComPtr<ID3D11ShaderResourceView> moveTexture_frame4;
	ComPtr<ID3D11ShaderResourceView> moveTexture_frame5;
	ComPtr<ID3D11ShaderResourceView> moveShootTexture_frame1;
	ComPtr<ID3D11ShaderResourceView> moveShootTexture_frame3;
	ComPtr<ID3D11ShaderResourceView> moveShootTexture_frame4;
	ComPtr<ID3D11ShaderResourceView> moveShootTexture_frame5;
	ComPtr<ID3D11ShaderResourceView> jumpTexture;
	ComPtr<ID3D11ShaderResourceView> jumpShootTexture;
	std::unique_ptr<Sprite> sprite;
	std::unique_ptr<Collider> collider;

	// states (probably want to move this to a playerController or something)
	float verticalVelocity{ 0.0f };
	bool jumpPressed{ false };
	bool jumpReleased{ true };
	bool canJump{ true };
	bool landed{ true };
	bool leftPressed{ false };
	bool rightPressed{ false };
	bool movingLeft{ false };
	bool movingRight{ false };
	bool canShoot{ true };
	bool shootPressed{ false };
	bool shooting{ false };

	// methods
	void Translate(const XMFLOAT2 vector);
	const CollisionResult CheckCollisionForPosition(const XMFLOAT2 proposedPos) const;
	
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
	virtual const void OnCollision(CollisionResult collisionResult);
	~Player();
};