#pragma once

#include "Event.h"
#include "../UI/UIComponent.h"

class Observer;

class EventHandler
{
	std::queue<std::unique_ptr<const Event>> eventQueue;
	// we're using a bidirectional linked list to optimize for add/remove due to the transient nature of game objects
	std::list<Observer*> observers;
public:
	void Subscribe(Observer& observer);
	void Unsubscribe(Observer& observer);
	void QueueEvent(std::unique_ptr<Event>& event);
	void PublishEvents(std::vector<UIComponent*>& uiComponents);
};