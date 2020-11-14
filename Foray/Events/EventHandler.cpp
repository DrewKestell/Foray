#include "../stdafx.h"
#include "EventHandler.h"

void EventHandler::Subscribe(Observer& observer)
{
	observers.push_back(&observer);
}

void EventHandler::Unsubscribe(Observer& observer)
{
	observers.remove(&observer);
}

void EventHandler::QueueEvent(std::unique_ptr<Event>& event)
{
	eventQueue.push(std::move(event));
}

void EventHandler::PublishEvents(std::vector<UIComponent*>& uiComponents)
{
	while (!eventQueue.empty())
	{
		auto event = std::move(eventQueue.front());
		eventQueue.pop();

		// first let the ui elements handle the event
		for (auto i = (int)uiComponents.size() - 1; i >= 0; i--)
		{
			uiComponents.at(i)->HandleEvent(event.get());
		}

		// next let game objects handle the event
		for (auto observer : observers)
		{
			observer->HandleEvent(event.get());
		}
	}
}