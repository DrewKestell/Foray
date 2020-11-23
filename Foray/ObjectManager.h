#pragma once

#include "GameObject.h"

class ObjectManager
{
private:
	std::list<GameObject> gameObjects;

public:
	GameObject& CreateGameObject();
	void DeleteGameObjectComponents(const unsigned int gameObjectId);
	void DeleteGameObject(const unsigned int gameObjectId);
	void UpdateGameObjects();
	void Clear();
};