#include "../stdafx.h"
#include "MapTile.h"

MapTile::MapTile(const unsigned int gameObjectId, const unsigned int textureId)
	: GameObjectId{ gameObjectId },
	  TextureId{ textureId }
{
}