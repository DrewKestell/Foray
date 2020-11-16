#include "stdafx.h"
#include "Game.h"
#include "Constants.h"
#include "ObjectManager.h"
#include "Events/EventHandler.h"
#include "Events/ChangeActiveLayerEvent.h"
#include "Events/FireProjectileEvent.h"
#include "Events/DestroyGameObjectEvent.h"
#include "Graphics/RenderingEngine.h"

// incoming globals
extern std::unique_ptr<EventHandler> g_eventHandler;
extern std::unique_ptr<ObjectManager> g_objectManager;
extern std::unique_ptr<RenderingEngine> g_renderingEngine;

// outgoing globals
float g_cameraPosX{ 0.0f };

Game::Game()
{
	g_eventHandler->Subscribe(*this);
}

void Game::Initialize()
{
	CreateStaticGeometry();
	CreatePlayer();
}

void Game::Tick()
{
	player->Update();
}

const void Game::HandleEvent(const Event* const event)
{
	const auto type = event->Type;
	switch (type)
	{
		case EventType::FireProjectile:
		{
			const auto derivedEvent = (FireProjectileEvent*)event;
			const auto position = derivedEvent->Position;
			const auto velocity = derivedEvent->Velocity;
			
			GameObject& gameObject = g_objectManager->CreateGameObject();
			gameObject.Position = position;

			RenderComponent& renderComponent = g_renderingEngine->CreateRenderComponent(gameObject.GameObjectId, 11, 2, position, 16.0f, 12.0f);
			gameObject.RenderComponent = &renderComponent;

			auto collider = new Collider(gameObject, ColliderType::Projectile, 16.0f, 12.0f, position, true);
			gameObject.Collider = collider;

			const auto projectileOnUpdate = [velocity](GameObject* gameObject)
			{
				gameObject->Translate(velocity);
			};

			const auto playerParam = player;
			const auto destroySelf = [playerParam](GameObject* gameObject)
			{
				if (Utility::IsOffScreen(playerParam->GetPosition(), gameObject->Collider->GetRect()))
				{
					std::unique_ptr<Event> e = std::make_unique<DestroyGameObjectEvent>(gameObject->GameObjectId);
					g_eventHandler->QueueEvent(e);
				}
			};
			gameObject.BehaviorComponents.push_back(BehaviorComponent{ projectileOnUpdate });
			gameObject.BehaviorComponents.push_back(BehaviorComponent{ destroySelf });

			break;
		}
	}
}

void Game::CreateStaticGeometry()
{
	wchar_t buffer[MAX_PATH];
	GetCurrentDirectory(sizeof(buffer), buffer);

	std::ifstream i;
	i.open("./World/static-geometry.json");

	json j;
	i >> j;

	const auto staticGeometry = j["staticGeometry"];

	blocksJson = staticGeometry["blocks"];
	for (auto i = 0; i < blocksJson.size(); i++)
	{
		const auto block = blocksJson[i];

		const auto left = block["left"].get<float>();
		const auto top = block["top"].get<float>();
		const auto right = block["right"].get<float>();
		const auto bottom = block["bottom"].get<float>();
		const auto textureId = block["textureId"].get<int>();

		const auto width = 50.0f;
		const auto height = 50.0f;
		const auto position = XMFLOAT2{ right - (width / 2), bottom - (height / 2) };

		GameObject& gameObject = g_objectManager->CreateGameObject();
		gameObject.Position = position;

		RenderComponent& renderComponent = g_renderingEngine->CreateRenderComponent(gameObject.GameObjectId, textureId, 1, position, width, height);
		gameObject.RenderComponent = &renderComponent;

		auto collider = new Collider(gameObject, ColliderType::StaticGeometry, width, height, position, true);
		gameObject.Collider = collider;
	}
}

void Game::CreatePlayer()
{
	GameObject& gameObject = g_objectManager->CreateGameObject();
	gameObject.Position = XMFLOAT2{ 700.0f, 200.0f };

	RenderComponent& renderComponent = g_renderingEngine->CreateRenderComponent(gameObject.GameObjectId, 0, 1, gameObject.Position, 95.0f, 80.0f);
	gameObject.RenderComponent = &renderComponent;

	auto collider = new Collider(gameObject, ColliderType::Player, 60.0f, 80.0f, gameObject.Position, true);
	gameObject.Collider = collider;

	player = new Player(gameObject);
}
