#pragma once

class MapTile
{
public:
	const unsigned int GameObjectId;
	const unsigned int TextureId;

	MapTile(const unsigned int gameObjectId, const unsigned int textureId);
};