#include "stdafx.h"
#include "ObjectManager.h"
#include "Physics/PhysicsEngine.h"
#include "Graphics/RenderingEngine.h"

extern std::unique_ptr<PhysicsEngine> g_physicsEngine; 
extern std::unique_ptr<RenderingEngine> g_renderingEngine;

GameObject& ObjectManager::CreateGameObject()
{
	gameObjects.push_back(GameObject{});

	return gameObjects.back();
}

bool GameObjectsEqual(GameObject& gameObject, const unsigned int gameObjectId)
{
	return gameObject.GameObjectId == gameObjectId;
}

void ObjectManager::DeleteGameObject(const unsigned int gameObjectId)
{
	for (const auto& gameObject : gameObjects)
	{
		if (gameObject.GameObjectId == gameObjectId)
		{
			if (gameObject.Collider)
			{
				g_physicsEngine->UnregisterCollider(gameObject.Collider);
			}

			if (gameObject.RenderComponent)
			{
				g_renderingEngine->RemoveRenderComponent(gameObject.GameObjectId);
			}

			gameObjects.remove(gameObject);

			return;
		}
	}
}

void ObjectManager::UpdateGameObjects()
{
	for (auto& gameObject : gameObjects)
	{
		for (auto behaviorComponent : gameObject.BehaviorComponents)
		{
			behaviorComponent.OnUpdate(&gameObject);
		}
	}
}