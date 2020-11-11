#include "stdafx.h"
#include "Game.h"
#include "Constants.h"
#include "Utility.h"
#include "Events/Event.h"
#include "Events/EventHandler.h"
#include "Events/Observer.h"
#include "Events/ChangeActiveLayerEvent.h"
#include "Events/GamepadInputEvent.h"
#include "Events/FireProjectileEvent.h"
#include "Physics/PhysicsEngine.h"
#include "Graphics/RenderingEngine.h"

float g_clientWidth{ CLIENT_WIDTH };
float g_clientHeight{ CLIENT_HEIGHT };
XMMATRIX g_projectionTransform{ XMMatrixIdentity() };
extern std::unique_ptr<EventHandler> g_eventHandler;
extern std::unique_ptr<PhysicsEngine> g_physicsEngine;
std::unique_ptr<RenderingEngine> g_renderingEngine;

Game::Game()
{
	g_eventHandler->Subscribe(*this);

	deviceResources = std::make_unique<DeviceResources>();
	deviceResources->RegisterDeviceNotify(this);
}

void Game::Tick()
{
	timer.Tick();

	updateTimer += timer.DeltaTime();
	if (updateTimer >= UPDATE_FREQUENCY)
	{
		PublishEvents();

		player->Update();

		for (auto& gameObject : gameObjects)
		{
			for (auto behaviorComponent : gameObject.BehaviorComponents)
			{
				behaviorComponent.OnUpdate(&gameObject);
			}
		}
		
		updateTimer -= UPDATE_FREQUENCY;
	}
	
	g_renderingEngine->DrawScene();
}

// TODO: can we move this to EventHandler?
void Game::PublishEvents()
{
	std::queue<std::unique_ptr<const Event>>& eventQueue = g_eventHandler->GetEventQueue();
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
		std::list<Observer*>& observers = g_eventHandler->GetObservers();
		for (auto observer : observers)
		{
			observer->HandleEvent(event.get());
		}
	}
}

const void Game::HandleEvent(const Event* const event)
{
	const auto type = event->type;
	switch (type)
	{
		case EventType::GamepadInput:
		{
			const auto derivedEvent = (GamepadInputEvent*)event;

			if (derivedEvent->inputValue == XINPUT_GAMEPAD_B)
			{
				if (activeLayer == Layer::MainOptions)
					SetActiveLayer(Layer::MainMenu);
			}

			break;
		}
		case EventType::FireProjectile:
		{
			const auto derivedEvent = (FireProjectileEvent*)event;
			const auto position = derivedEvent->position;
			const auto velocity = derivedEvent->velocity;
			
			gameObjects.push_back(GameObject{});
			GameObject& gameObject = gameObjects.back();
			gameObject.Position = position;

			RenderComponent& renderComponent = g_renderingEngine->CreateRenderComponent(gameObject.GameObjectId, 11, 3, position, 16.0f, 12.0f);
			gameObject.RenderComponent = &renderComponent;

			auto collider = new Collider(gameObject, ColliderType::Projectile, 16.0f, 12.0f, position, true);
			gameObject.Collider = collider;

			const auto projectileOnUpdate = [velocity](GameObject* gameObject)
			{
				gameObject->Translate(velocity);
			};
			gameObject.BehaviorComponents.push_back(BehaviorComponent{ projectileOnUpdate });

			break;
		}
	}
}

// TODO: do we really need this?
void Game::SetActiveLayer(const Layer layer)
{
	activeLayer = layer;

	std::unique_ptr<Event> e = std::make_unique<ChangeActiveLayerEvent>(layer);
	g_eventHandler->QueueEvent(e);
}

// TODO: put this inside some new "Shader" class
ShaderBuffer Game::LoadShader(const std::wstring filename)
{
	// load precompiled shaders from .cso objects
	ShaderBuffer sb{ nullptr, 0 };
	byte* fileData{ nullptr };

	// open the file
	std::ifstream csoFile(filename, std::ios::in | std::ios::binary | std::ios::ate);

	if (csoFile.is_open())
	{
		// get shader size
		sb.size = (unsigned int)csoFile.tellg();

		// collect shader data
		fileData = new byte[sb.size];
		csoFile.seekg(0, std::ios::beg);
		csoFile.read(reinterpret_cast<char*>(fileData), sb.size);
		csoFile.close();
		sb.buffer = fileData;
	}
	else
		throw std::exception("Critical error: Unable to open the compiled shader object!");

	return sb;
}

void Game::Initialize(const HWND window, const int width, const int height)
{
	deviceResources->SetWindow(window, width, height);

	CreateUIElements();
	CreateStaticGeometry();

	deviceResources->CreateDeviceResources();
	deviceResources->CreateWindowSizeDependentResources();

	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();

	timer.Reset();
	SetActiveLayer(Layer::Game);
}

void Game::CreateUIElements()
{
	wchar_t buffer[MAX_PATH];
	GetCurrentDirectory(sizeof(buffer), buffer);

	std::ifstream i;
	i.open("./UI/ui-elements.json");

	json j;
	i >> j;

	const auto uiElements = j["uiElements"];

	brushesJson = uiElements["brushes"];
	for (auto i = 0; i < brushesJson.size(); i++)
	{
		const auto brush = brushesJson[i];
		const auto id = brush["id"].get<std::string>();
		brushes[id] = ComPtr<ID2D1SolidColorBrush>();
	}

	textFormatsJson = uiElements["textFormats"];
	for (auto i = 0; i < textFormatsJson.size(); i++)
	{
		const auto textFormat = textFormatsJson[i];
		const auto id = textFormat["id"].get<std::string>();
		textFormats[id] = ComPtr<IDWriteTextFormat>();
	}

	labelsJson = uiElements["labels"];
	for (auto i = 0; i < labelsJson.size(); i++)
	{
		const auto label = labelsJson[i];
		const auto id = label["id"].get<std::string>();
		const auto x = label["x"].get<float>();
		const auto y = label["y"].get<float>();
		const auto layer = LAYER_MAP.at(label["layer"].get<std::string>());
		const auto zIndex = label["zIndex"].get<unsigned int>();
		labels[id] = std::make_unique<UILabel>(UIComponentArgs{ deviceResources.get(), uiComponents, [x, y](const float, const float) { return XMFLOAT2{ x, y }; }, layer, zIndex });
	}

	menuItemGroupsJson = uiElements["menuItemGroups"];
	for (auto i = 0; i < menuItemGroupsJson.size(); i++)
	{
		const auto menuItemGroupJson = menuItemGroupsJson[i];
		const auto menuItemGroupId = menuItemGroupJson["id"].get<std::string>();
		
		menuItemGroups[menuItemGroupId] = std::make_unique<UIMenuItemGroup>(Layer::MainMenu);

		const auto menuItemsJson = menuItemGroupJson["menuItems"];

		for (auto j = 0; j < menuItemsJson.size(); j++)
		{
			const auto menuItem = menuItemsJson[j];
			const auto menuItemId = menuItem["id"].get<std::string>();
			const auto x = menuItem["x"].get<float>();
			const auto y = menuItem["y"].get<float>();
			const auto layer = LAYER_MAP.at(menuItem["layer"].get<std::string>());
			const auto zIndex = menuItem["zIndex"].get<unsigned int>();
			const auto text = menuItem["text"].get<std::string>();
			const auto isActive = menuItem["isActive"].get<bool>();

			std::function<void()> onActivate;
			if (menuItemId == "menuScreen_start")
			{
				onActivate = [this]()
				{
					SetActiveLayer(Layer::Game);
				};
			}
			else if (menuItemId == "menuScreen_options")
			{
				onActivate = [this]()
				{
					SetActiveLayer(Layer::MainOptions);
				};
			}
			else if (menuItemId == "menuScreen_mainOptionsBack")
			{
				onActivate = [this]()
				{
					SetActiveLayer(Layer::MainMenu);
				};
			}

			menuItems[menuItemId] = std::make_unique<UIMenuItem>(UIComponentArgs{ deviceResources.get(), uiComponents, [x, y](const float, const float) { return XMFLOAT2{ x, y }; }, layer, zIndex }, text.c_str(), onActivate, isActive);
			menuItemGroups[menuItemGroupId]->AddInput(menuItems[menuItemId].get());
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

		const auto id = block["id"].get<std::string>();
		const auto left = block["left"].get<float>();
		const auto top = block["top"].get<float>();
		const auto right = block["right"].get<float>();
		const auto bottom = block["bottom"].get<float>();
		const auto radiusX = block["radiusX"].get<float>();
		const auto radiusY = block["radiusY"].get<float>();

		auto gameObject = new GameObject();

		const auto width = right - left;
		const auto height = bottom - top;
		const auto position = XMFLOAT2{ right - (width / 2), bottom - (height / 2) };
		auto collider = new Collider(*gameObject, ColliderType::StaticGeometry, width, height, position, true);

		blocks[id] = std::make_unique<Block>(D2D1::RoundedRect(D2D1::RectF(left, top, right, bottom), radiusX, radiusY), gameObject, collider);
	}
}

void Game::CreateDeviceDependentResources()
{
	// UI Elements
	InitializeShaders();
	InitializeTextures();
	InitializeBrushes();
	InitializeTextFormats();
	InitializeLabels();
	InitializeMenuItems();

	g_renderingEngine = std::make_unique<RenderingEngine>(deviceResources.get(), uiComponents, blocks, textures, spriteVertexShader.Get(), spritePixelShader.Get(), spriteVertexShaderBuffer.buffer, spriteVertexShaderBuffer.size);

	// Static Geometry
	InitializeBlocks();
}

void Game::CreateWindowSizeDependentResources()
{
	g_projectionTransform = XMMatrixOrthographicLH(g_clientWidth, g_clientHeight, 0.0f, 5000.0f);

	InitializePlayer();
}

void Game::InitializeShaders()
{
	auto d3dDevice = deviceResources->GetD3DDevice();

	spriteVertexShaderBuffer = LoadShader(L"SpriteVertexShader.cso");
	d3dDevice->CreateVertexShader(spriteVertexShaderBuffer.buffer, spriteVertexShaderBuffer.size, nullptr, spriteVertexShader.ReleaseAndGetAddressOf());

	spritePixelShaderBuffer = LoadShader(L"SpritePixelShader.cso");
	d3dDevice->CreatePixelShader(spritePixelShaderBuffer.buffer, spritePixelShaderBuffer.size, nullptr, spritePixelShader.ReleaseAndGetAddressOf());
}

void Game::InitializeTextures()
{
	auto d3dDevice = deviceResources->GetD3DDevice();

	const std::vector<const wchar_t*> paths
	{
		L"./Graphics/Textures/megaman_moveTexture_frame1.DDS",      // 0
		L"./Graphics/Textures/megaman_moveTexture_frame2.DDS",      // 1
		L"./Graphics/Textures/megaman_moveTexture_frame3.DDS",      // 2
		L"./Graphics/Textures/megaman_moveTexture_frame4.DDS",      // 3
		L"./Graphics/Textures/megaman_moveTexture_frame5.DDS",      // 4
		L"./Graphics/Textures/megaman_moveShootTexture_frame1.DDS", // 5
		L"./Graphics/Textures/megaman_moveShootTexture_frame3.DDS", // 6
		L"./Graphics/Textures/megaman_moveShootTexture_frame4.DDS", // 7
		L"./Graphics/Textures/megaman_moveShootTexture_frame5.DDS", // 8
		L"./Graphics/Textures/megaman_jumpTexture.DDS",             // 9
		L"./Graphics/Textures/megaman_jumpShootTexture.DDS",        // 10
		L"./Graphics/Textures/bullet.DDS",                          // 11
	};

	// clear calls the destructor of its elements, and ComPtr's destructor handles calling Release()
	textures.clear();

	for (auto i = 0; i < paths.size(); i++)
	{
		ComPtr<ID3D11ShaderResourceView> ptr;
		CreateDDSTextureFromFile(d3dDevice, paths.at(i), nullptr, ptr.ReleaseAndGetAddressOf());
		textures.push_back(ptr);
	}
}

void Game::InitializePlayer()
{
	gameObjects.push_back(GameObject{});
	GameObject& gameObject = gameObjects.back();
	gameObject.Position = XMFLOAT2{ 200.0f, 200.0f };

	RenderComponent& renderComponent = g_renderingEngine->CreateRenderComponent(gameObject.GameObjectId, 0, 3, gameObject.Position, 95.0f, 80.0f);
	gameObject.RenderComponent = &renderComponent;

	auto collider = new Collider(gameObject, ColliderType::Player, 60.0f, 80.0f, gameObject.Position, true);
	gameObject.Collider = collider;

	player = new Player(gameObject);
}

void Game::InitializeBrushes()
{
	auto d2dContext = deviceResources->GetD2DDeviceContext();

	for (auto i = 0; i < brushesJson.size(); i++)
	{
		const auto brush = brushesJson[i];
		const auto id = brush["id"].get<std::string>();
		const auto r = brush["r"].get<float>();
		const auto g = brush["g"].get<float>();
		const auto b = brush["b"].get<float>();
		const auto a = brush["a"].get<float>();

		d2dContext->CreateSolidColorBrush(D2D1::ColorF(r, g, b, a), brushes[id].ReleaseAndGetAddressOf());
	}
}

void Game::InitializeTextFormats()
{
	auto writeFactory = deviceResources->GetWriteFactory();

	for (auto i = 0; i < textFormatsJson.size(); i++)
	{
		const auto textFormat = textFormatsJson[i];
		const auto id = textFormat["id"].get<std::string>();
		const auto fontFamily = Utility::s2ws(textFormat["fontFamily"].get<std::string>());
		const auto weight = FONT_WEIGHT_MAP.at(textFormat["weight"].get<std::string>());
		const auto style = FONT_STYLE_MAP.at(textFormat["style"].get<std::string>());
		const auto stretch = FONT_STRETCH_MAP.at(textFormat["stretch"].get<std::string>());
		const auto size = textFormat["size"].get<float>();
		const auto textAlignment = TEXT_ALIGNMENT_MAP.at(textFormat["textAlignment"].get<std::string>());
		const auto paragraphAlignment = PARAGRAPH_ALIGNMENT_MAP.at(textFormat["paragraphAlignment"].get<std::string>());

		writeFactory->CreateTextFormat(fontFamily.c_str(), nullptr, weight, style, stretch, size, LOCALE, textFormats.at(id).ReleaseAndGetAddressOf());
		textFormats.at(id)->SetTextAlignment(textAlignment);
		textFormats.at(id)->SetParagraphAlignment(paragraphAlignment);
	}
}

void Game::InitializeLabels()
{
	for (auto i = 0; i < labelsJson.size(); i++)
	{
		const auto labelJson = labelsJson[i];
		const auto id = labelJson["id"].get<std::string>();
		const auto label = labels.at(id).get();
		const auto brushId = labelJson["brushId"].get<std::string>();
		const auto textFormatId = labelJson["textFormatId"].get<std::string>();
		const auto initialText = Utility::s2ws(labelJson["initialText"].get<std::string>());
		label->Initialize(brushes.at(brushId).Get(), textFormats.at(textFormatId).Get());
		label->SetText(initialText.c_str());
	}
}

void Game::InitializeMenuItems()
{
	for (auto i = 0; i < menuItemGroupsJson.size(); i++)
	{
		const auto menuItemsJson = menuItemGroupsJson[i]["menuItems"];

		for (auto j = 0; j < menuItemsJson.size(); j++)
		{
			const auto menuItem = menuItemsJson[j];
			const auto id = menuItem["id"].get<std::string>();
			const auto brushId = menuItem["brushId"].get<std::string>();
			const auto bodyTextFormatId = menuItem["bodyTextFormatId"].get<std::string>();
			const auto bulletTextFormatId = menuItem["bulletTextFormatId"].get<std::string>();

			menuItems.at(id)->Initialize(brushes.at(brushId).Get(), textFormats.at(bodyTextFormatId).Get(), textFormats.at(bulletTextFormatId).Get());
		}
	}
}

void Game::InitializeBlocks()
{
	auto d2dContext = deviceResources->GetD2DDeviceContext();
	auto d2dFactory = deviceResources->GetD2DFactory();

	for (auto i = 0; i < blocksJson.size(); i++)
	{
		const auto blockJson = blocksJson[i];

		const auto id = blockJson["id"].get<std::string>();
		const auto block = blocks.at(id).get();
		const auto fillBrushId = blockJson["fillBrushId"].get<std::string>();
		
		block->Initialize(d2dContext, d2dFactory, brushes.at(fillBrushId).Get());
	}
}

void Game::OnWindowSizeChanged(const int width, const int height)
{
	if (!deviceResources->WindowSizeChanged(width, height))
		return;

	g_clientWidth = static_cast<float>(width);
	g_clientHeight = static_cast<float>(height);

	CreateWindowSizeDependentResources();
}

void Game::OnWindowMoved()
{
	const auto r = deviceResources->GetOutputSize();
	deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnDeviceLost()
{
	// TODO: Add Direct3D resource cleanup here.
	// may not be needed - classes should be designed with RAII in mind, and clean up
	// after themselves. not sure if there is an exception to this with D3D,
	// so i'm leaving this stubbed out for now.
}

void Game::OnDeviceRestored()
{
	CreateDeviceDependentResources();

	CreateWindowSizeDependentResources();

	SetActiveLayer(activeLayer);
}
