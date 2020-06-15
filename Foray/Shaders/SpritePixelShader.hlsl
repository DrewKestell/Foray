Texture2D GTexture : register(t0);
SamplerState GSampler : register(s0);

cbuffer cbPerObject : register(b0)
{
	bool gMirrorHorizontal;
	int pack1;
	int pack2;
	int pack3;
};

struct PixelInput
{
	float4 Pos : SV_POSITION;
	float2 TexCoords : TEXCOORDS;
};

float4 main(PixelInput pin) : SV_TARGET
{
	float x = gMirrorHorizontal ? -pin.TexCoords.x : pin.TexCoords.x;
	return GTexture.Sample(GSampler, float2(x, pin.TexCoords.y));
}