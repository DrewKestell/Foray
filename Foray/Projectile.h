#pragma once

#include <memory>
#include "UI/Layer.h"
#include "Events/Observer.h"
#include "GameObject.h"
#include "Sprite.h"
#include "Physics/Collider.h"

class Projectile : public Observer, public GameObject
{
private:
	// constructor params
	const unsigned int ownerId;
	XMFLOAT2 position;
	XMFLOAT2 velocity;

	// device dependent resources
	ID3D11VertexShader* vertexShader{ nullptr };
	ID3D11PixelShader* pixelShader{ nullptr };
	const BYTE* vertexShaderBuffer{ 0 };
	int vertexShaderSize{ 0 };
	ID3D11Device* device{ nullptr };

	// locals
	std::unique_ptr<Sprite> sprite;
	std::unique_ptr<Collider> collider;
	ComPtr<ID3D11ShaderResourceView> bulletTexture;
	Layer activeLayer{ Layer::Game };

	// methods
	void Translate(const XMFLOAT2 vector);
	
public:
	Projectile(const unsigned int ownerId, const XMFLOAT2 position, const XMFLOAT2 velocity);
	void Initialize(
		ID3D11VertexShader* vertexShader,
		ID3D11PixelShader* pixelShader,
		const BYTE* vertexShaderBuffer,
		const int vertexShaderSize,
		ID3D11Device* device);
	void Update();
	void Draw(ID3D11DeviceContext* d3dContext);
	const D2D1_RECT_F GetBoundingBox() const;
	virtual const void HandleEvent(const Event* const event);
	virtual const void OnCollision(CollisionResult collisionResult);
	~Projectile();
};