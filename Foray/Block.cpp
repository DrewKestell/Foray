#include "stdafx.h"
#include "Block.h"
#include "Physics/PhysicsEngine.h"

extern std::unique_ptr<PhysicsEngine> g_physicsEngine;

Block::Block(EventHandler& eventHandler, const D2D1_ROUNDED_RECT rect)
	: eventHandler{ eventHandler },
	  rect{ rect },
	  collider{ std::make_unique<Collider>(eventHandler, rect.rect) }
{
	eventHandler.Subscribe(*this);
	g_physicsEngine->RegisterCollider(collider.get());
}

void Block::Initialize(ID2D1DeviceContext1* d2dContext, ID2D1Factory2* d2dFactory, ID2D1SolidColorBrush* fillBrush)
{
	this->d2dContext = d2dContext;
	this->fillBrush = fillBrush;

	d2dFactory->CreateRoundedRectangleGeometry(rect, geometry.ReleaseAndGetAddressOf());
}

void Block::Draw()
{
	d2dContext->FillGeometry(geometry.Get(), fillBrush);
}

const void Block::HandleEvent(const Event* const event)
{
	const auto type = event->type;
	switch (type)
	{

	}
}


Block::~Block()
{
	eventHandler.Unsubscribe(*this);
	g_physicsEngine->UnregisterCollider(collider.get());
}