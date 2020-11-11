#pragma once

extern float g_clientWidth;
extern float g_clientHeight;
extern XMMATRIX g_projectionTransform;

class RenderComponent
{
private:
	

public:
	const float Width;
	const float Height;
	unsigned int TextureId;
	const unsigned int ZIndex;
	XMFLOAT2 Position;
	bool MirrorHorizontal{ false };

	RenderComponent(
		const unsigned int textureId,
		const unsigned int zIndex,
		const XMFLOAT2 position,
		const float width,
		const float height);
	void SetPosition(const XMFLOAT2 pos);
};