#include "../stdafx.h"
#include "Editor.h"
#include "../ObjectManager.h"
#include "../Events/EventHandler.h"
#include "../Events/KeyDownEvent.h"
#include "../Events/KeyUpEvent.h"
#include "../Events/MouseEvent.h"
#include "../Events/ChangeActiveLayerEvent.h"
#include "../Graphics/RenderingEngine.h"

// incoming globals
extern std::unique_ptr<ObjectManager> g_objectManager;
extern std::unique_ptr<RenderingEngine> g_renderingEngine;
extern std::unique_ptr<EventHandler> g_eventHandler;

Editor::Editor(UIEditorToolButtonGroup* toolButtonGroup, UIEditorTexturePicker* texturePicker)
	: toolButtonGroup{ toolButtonGroup },
	  texturePicker{ texturePicker }
{
	g_eventHandler->Subscribe(*this);
}

void Editor::Initialize()
{

}

void Editor::Tick()
{

}

const bool Editor::HandleEvent(const Event* const event)
{
	const auto type = event->Type;
	switch (type)
	{
		case EventType::LeftMouseDown:
		{
			if (activeLayer == Layer::Editor)
			{
				const auto mouseDownEvent = (MouseEvent*)event;

				const auto worldPos = Utility::ConvertToWorldSpace(XMFLOAT2{ mouseDownEvent->MousePosX, mouseDownEvent->MousePosY });
				auto tileX = floor(worldPos.x / 50.0f);
				auto tileY = floor(worldPos.y / 50.0f);

				std::cout << "X: " << worldPos.x << std::endl;
				std::cout << "Y: " << worldPos.y << std::endl;
				std::cout << "TileX: " << tileX << std::endl;
				std::cout << "TileY: " << tileY << std::endl;

				const auto activeTool = toolButtonGroup->GetActiveTool();
				if (activeTool == L"+")
					CreateTile(tileX, tileY);
				else if (activeTool == L"-")
					RemoveTile(tileX, tileY);
			}
			
			break;
		}
		case EventType::KeyDown:
		{
			const auto derivedEvent = (KeyDownEvent*)event;

			

			break;
		}
		case EventType::KeyUp:
		{
			const auto derivedEvent = (KeyUpEvent*)event;

			

			break;
		}
		case EventType::ChangeActiveLayer:
		{
			const auto derivedEvent = (ChangeActiveLayerEvent*)event;

			activeLayer = derivedEvent->Layer;

			break;
		}
	}

	return false;
}

void Editor::Initialize(const HWND window, const int width, const int height)
{

}

void Editor::ClearTiles()
{
	for (auto i = 0; i < 1000; i++)
	{
		for (auto j = 0; j < 1000; j++)
		{
			mapTiles[i][j].reset();
		}
	}
}	

void Editor::CreateTile(const int x, const int y)
{
	if (mapTiles[x][y])
		RemoveTile(x, y);

	const auto textureId = texturePicker->GetActiveTextureId();

	const auto position = XMFLOAT2{ (x * 50.0f) + 25.0f, (y * 50.0f) + 25.0f };
	GameObject& gameObject = g_objectManager->CreateGameObject();
	gameObject.Position = position;

	std::cout << "Final PosX: " << position.x << std::endl;
	std::cout << "Final PosY: " << position.y << std::endl;

	RenderComponent& renderComponent = g_renderingEngine->CreateRenderComponent(gameObject.GameObjectId, textureId, 1, position, 50.0f, 50.0f, false);
	gameObject.RenderComponent = &renderComponent;

	mapTiles[x][y] = std::make_unique<MapTile>(gameObject.GameObjectId, textureId);
}

void Editor::RemoveTile(const int x, const int y)
{
	if (mapTiles[x][y])
	{
		g_objectManager->DeleteGameObjectComponents(mapTiles[x][y]->GameObjectId);
		g_objectManager->DeleteGameObject(mapTiles[x][y]->GameObjectId);

		mapTiles[x][y].reset();
	}
}