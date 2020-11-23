#include "../stdafx.h"
#include "Editor.h"
#include "../ObjectManager.h"
#include "../Events/EventHandler.h"
#include "../Events/KeyDownEvent.h"
#include "../Events/KeyUpEvent.h"
#include "../Events/MouseEvent.h"

// incoming globals
extern std::unique_ptr<ObjectManager> g_objectManager;
extern std::unique_ptr<EventHandler> g_eventHandler;

Editor::Editor()
{
	g_eventHandler->Subscribe(*this);
}

void Editor::Initialize()
{

}

void Editor::Tick()
{

}

const void Editor::HandleEvent(const Event* const event)
{
	const auto type = event->Type;
	switch (type)
	{
		case EventType::LeftMouseDown:
		{
			const auto mouseDownEvent = (MouseEvent*)event;

			std::cout << "X: " << mouseDownEvent->MousePosX << std::endl;
			std::cout << "Y: " << mouseDownEvent->MousePosY << std::endl;

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
	}
}

void Editor::Initialize(const HWND window, const int width, const int height)
{

}