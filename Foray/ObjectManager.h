#pragma once

#include "GameObject.h"

class ObjectManager
{
private:
	std::list<GameObject> gameObjects;

public:
	GameObject& CreateGameObject();
	void DeleteGameObject(const unsigned int gameObjectId);
	void UpdateGameObjects();
};