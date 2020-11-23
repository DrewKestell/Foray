#pragma once

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
	bool Visible{ true };

	RenderComponent(
		const unsigned int textureId,
		const unsigned int zIndex,
		const XMFLOAT2 position,
		const float width,
		const float height,
		const bool convertToWorldPos);
	void SetPosition(const XMFLOAT2 pos);
};