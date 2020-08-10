#pragma once

#include <d2d1_3.h>
#include <wrl/client.h>
#include "Physics/Collider.h"
#include "Events/Observer.h"
#include "Events/EventHandler.h"

using namespace Microsoft::WRL;

class Block : public Observer
{
private:
	// constructor params
	EventHandler& eventHandler;
	D2D1_ROUNDED_RECT rect;

	// locals
	std::unique_ptr<Collider> collider;
	ID2D1DeviceContext1* d2dContext{ nullptr };
	ID2D1SolidColorBrush* fillBrush{ nullptr };
	ComPtr<ID2D1RoundedRectangleGeometry> geometry;

public:
	Block(EventHandler& eventHandler, const D2D1_ROUNDED_RECT rect);
	void Initialize(ID2D1DeviceContext1* d2dContext, ID2D1Factory2* d2dFactory, ID2D1SolidColorBrush* fillBrush);
	void Draw();
	virtual const void HandleEvent(const Event* const event);
	~Block();
};