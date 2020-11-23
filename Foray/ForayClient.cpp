#include "stdafx.h"
#include "ForayClient.h"
#include "../Events/EventHandler.h"
#include "../Constants.h"
#include "../Events/KeyDownEvent.h"
#include "../Events/KeyUpEvent.h"
#include "../Events/DestroyGameObjectEvent.h"
#include "../Events/ChangeActiveLayerEvent.h"
#include "../Graphics/RenderingEngine.h"
#include "../Events/GamepadInputEvent.h"
#include "Sound/SoundEngine.h"
#include "Physics/PhysicsEngine.h"
#include "../ObjectManager.h"

// incoming globals
extern std::unique_ptr<EventHandler> g_eventHandler;

// outgoing globals
unsigned int g_zIndex{ 0 };
float g_clientWidth{ CLIENT_WIDTH };
float g_clientHeight{ CLIENT_HEIGHT };
XMMATRIX g_projectionTransform{ XMMatrixIdentity() };
std::unique_ptr<ObjectManager> g_objectManager;
std::unique_ptr<PhysicsEngine> g_physicsEngine;
std::unique_ptr<SoundEngine> g_soundEngine;
std::unique_ptr<RenderingEngine> g_renderingEngine;

ForayClient::ForayClient(const HWND window, const int width, const int height)
{
	g_eventHandler->Subscribe(*this);

	deviceResources = std::make_unique<DeviceResources>();
	deviceResources->RegisterDeviceNotify(this);

	g_objectManager = std::make_unique<ObjectManager>();
	g_physicsEngine = std::make_unique<PhysicsEngine>();
	g_soundEngine = std::make_unique<SoundEngine>();
	g_renderingEngine = std::make_unique<RenderingEngine>(deviceResources.get(), uiComponents, textures);

	deviceResources->SetWindow(window, width, height);

	CreateUIElements();

	deviceResources->CreateDeviceResources();
	deviceResources->CreateWindowSizeDependentResources();

	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();

	game = std::make_unique<Game>();
	editor = std::make_unique<Editor>();

	timer.Reset();
}

void ForayClient::Tick()
{
	timer.Tick();

	updateTimer += timer.DeltaTime();
	while (updateTimer >= UPDATE_FREQUENCY)
	{
		g_eventHandler->PublishEvents(uiComponents);
		g_objectManager->UpdateGameObjects();

		if (activeLayer == Layer::Game)
			game->Tick();
		else if (activeLayer == Layer::Editor)
			editor->Tick();

		updateTimer -= UPDATE_FREQUENCY;
	}

	g_renderingEngine->DrawScene();
}

const void ForayClient::HandleEvent(const Event* const event)
{
	const auto type = event->Type;
	switch (type)
	{
		case EventType::ChangeActiveLayer:
		{
			const auto derivedEvent = (ChangeActiveLayerEvent*)event;

			activeLayer = derivedEvent->Layer;

			g_objectManager->Clear();

			if (activeLayer == Layer::Game)
			{
				game->Initialize();
			}
			else if (activeLayer == Layer::Editor)
			{
				// TODO: move this into editor->Initialize?
				const auto texturePickerX = 5.0f;
				const auto texturePickerY = 50.0f;
				const auto position = XMFLOAT2{ 20.0f + texturePickerX + 26.0f, 20.0f + texturePickerY + 26.0f };

				GameObject& texturePickerGameObject = g_objectManager->CreateGameObject();
				texturePickerGameObject.Position = position;

				RenderComponent& renderComponent = g_renderingEngine->CreateRenderComponent(texturePickerGameObject.GameObjectId, 12, 2, position, 50.0f, 50.0f);
				renderComponent.Visible = false;
				texturePickerGameObject.RenderComponent = &renderComponent;

				const std::string labelName = "editorToolsPanelTitle";
				labels[labelName]->Initialize(brushes.at("white").Get(), textFormats.at("panelTitle").Get());
				labels[labelName]->SetText(L"Editor Tools");
				editorTexturePicker->Initialize(&texturePickerGameObject, brushes.at("gray").Get());

				editor->Initialize();
			}

			break;
		}
		case EventType::GamepadInput:
		{
			const auto derivedEvent = (GamepadInputEvent*)event;

			if (derivedEvent->InputValue == XINPUT_GAMEPAD_B)
			{
				if (activeLayer == Layer::MainOptions)
					SetActiveLayer(Layer::MainMenu);
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
		case EventType::DestroyGameObject:
		{
			const auto derivedEvent = (DestroyGameObjectEvent*)event;

			g_objectManager->DeleteGameObjectComponents(derivedEvent->GameObjectId);
			g_objectManager->DeleteGameObject(derivedEvent->GameObjectId);

			break;
		}
	}
}

void ForayClient::SetActiveLayer(const Layer layer)
{
	activeLayer = layer;

	g_objectManager->Clear();

	std::unique_ptr<Event> e = std::make_unique<ChangeActiveLayerEvent>(layer);
	g_eventHandler->QueueEvent(e);
}

void ForayClient::CreateUIElements()
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
			else if (menuItemId == "menuScreen_editor")
			{
				onActivate = [this]()
				{
					SetActiveLayer(Layer::Editor);
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

	panelsJson = uiElements["panels"];
	for (auto i = 0; i < panelsJson.size(); i++)
	{
		const auto panel = panelsJson[i];

		const auto id = panel["id"].get<std::string>();
		const auto layer = LAYER_MAP.at(panel["layer"].get<std::string>());
		const auto isDraggable = panel["isDraggable"].get<bool>();
		const auto width = panel["width"].get<float>();
		const auto height = panel["height"].get<float>();
		const auto x = panel["x"].get<float>() - (width / 2);
		const auto y = panel["y"].get<float>() - (height / 2);
		const auto showKey = panel["showKey"].get<int>();
		
		panels[id] = std::make_unique<UIPanel>(UIComponentArgs{ deviceResources.get(), uiComponents, [x, y](const float, const float) { return XMFLOAT2{ x, y }; }, layer, 0 }, isDraggable, width, height, showKey);
	
		const auto buttonsJson = panel["buttons"];

		for (auto j = 0; j < buttonsJson.size(); j++)
		{
			const auto button = buttonsJson[j];

			const auto buttonId = button["id"].get<std::string>();
			const auto buttonLayer = LAYER_MAP.at(button["layer"].get<std::string>());
			const auto buttonWidth = button["width"].get<float>();
			const auto buttonHeight = button["height"].get<float>();
			const auto buttonX = button["x"].get<float>();
			const auto buttonY = button["y"].get<float>();
			const auto buttonText = Utility::s2ws(button["buttonText"].get<std::string>());

			std::function<void()> onClick;
			if (buttonId == "inGameOptions_leaveGame")
			{
				onClick = [this]()
				{
					// clean up game and return to title screen
					SetActiveLayer(Layer::MainMenu);
				};
			}
			if (buttonId == "editorOptions_leaveGame")
			{
				onClick = [this]()
				{
					// clean up game and return to title screen
					SetActiveLayer(Layer::MainMenu);
				};
			}

			buttons[buttonId] = std::make_unique<UIButton>(UIComponentArgs{ deviceResources.get(), uiComponents, [buttonX, buttonY](const float, const float) { return XMFLOAT2{ buttonX, buttonY }; }, buttonLayer, 1 }, buttonWidth, buttonHeight, buttonText.c_str(), onClick);
		
			panels[id]->AddChildComponent(*buttons[buttonId]);
		}

		// hard code certain elements for the Editor panel. TODO: is there a better way to do this?

		if (id == "editorToolbox")
		{
			editorToolButtonGroups[id] = std::make_unique<UIEditorToolButtonGroup>(Layer::Editor);

			const auto labelX = 5.0f;
			const auto labelY = 4.0f;
			const std::string labelName = "editorToolsPanelTitle";
			labels[labelName] = std::make_unique<UILabel>(UIComponentArgs{ deviceResources.get(), uiComponents, [labelX, labelY](const float, const float) { return XMFLOAT2{ labelX, labelY }; }, layer, 1 });
			panels[id]->AddChildComponent(*labels[labelName]);

			const auto tool1ButtonX = 5.0f;
			const auto tool1ButtonY = 25.0f;
			const std::string tool1Name = "addTile";
			editorToolButtons[tool1Name] = std::make_unique<UIEditorToolButton>(UIComponentArgs{ deviceResources.get(), uiComponents, [tool1ButtonX, tool1ButtonY](const float, const float) { return XMFLOAT2{ tool1ButtonX, tool1ButtonY }; }, Layer::Editor, 1 }, L"+", editorToolButtonGroups[id].get());

			editorToolButtonGroups[id]->AddButton(editorToolButtons[tool1Name].get());
			panels[id]->AddChildComponent(*editorToolButtons[tool1Name]);

			const auto tool2ButtonX = 30.0f;
			const auto tool2ButtonY = 25.0f;
			const std::string tool2Name = "removeTile";
			editorToolButtons[tool2Name] = std::make_unique<UIEditorToolButton>(UIComponentArgs{ deviceResources.get(), uiComponents, [tool2ButtonX, tool2ButtonY](const float, const float) { return XMFLOAT2{ tool2ButtonX, tool2ButtonY }; }, Layer::Editor, 1 }, L"-", editorToolButtonGroups[id].get());

			editorToolButtonGroups[id]->AddButton(editorToolButtons[tool2Name].get());
			panels[id]->AddChildComponent(*editorToolButtons[tool2Name]);

			const auto texturePickerX = 5.0f;
			const auto texturePickerY = 50.0f;
			editorTexturePicker = std::make_unique<UIEditorTexturePicker>(UIComponentArgs{ deviceResources.get(), uiComponents, [texturePickerX, texturePickerY](const float, const float) { return XMFLOAT2{ texturePickerX, texturePickerY }; }, Layer::Editor, 1 });
			panels[id]->AddChildComponent(*editorTexturePicker);
		}
	}
}

void ForayClient::CreateDeviceDependentResources()
{
	InitializeTextures();
	InitializeBrushes();
	InitializeTextFormats();
	InitializeLabels();
	InitializeMenuItems();
	InitializePanels();
	InitializeEditorToolButtons();
	g_renderingEngine->Initialize();
}

void ForayClient::CreateWindowSizeDependentResources()
{
	g_projectionTransform = XMMatrixOrthographicLH(g_clientWidth, g_clientHeight, 0.0f, 5000.0f);
}

void ForayClient::InitializeTextures()
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
		L"./Graphics/Textures/tile_1.DDS",                          // 12
		L"./Graphics/Textures/tile_2.DDS",                          // 13
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

void ForayClient::InitializeBrushes()
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

void ForayClient::InitializeTextFormats()
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

void ForayClient::InitializeLabels()
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

void ForayClient::InitializeMenuItems()
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

void ForayClient::InitializePanels()
{
	for (auto i = 0; i < panelsJson.size(); i++)
	{
		const auto panel = panelsJson[i];

		const auto id = panel["id"].get<std::string>();
		const auto headerBrushId = panel["headerBrushId"].get<std::string>();
		const auto bodyBrushId = panel["bodyBrushId"].get<std::string>();
		const auto borderBrushId = panel["borderBrushId"].get<std::string>();

		panels.at(id)->Initialize(brushes.at(headerBrushId).Get(), brushes.at(bodyBrushId).Get(), brushes.at(borderBrushId).Get());
	
		const auto buttonsJson = panelsJson[i]["buttons"];

		for (auto j = 0; j < buttonsJson.size(); j++)
		{
			const auto button = buttonsJson[j];

			const auto buttonId = button["id"].get<std::string>();
			const auto buttonBrushId = button["brushId"].get<std::string>();
			const auto pressedButtonBrushId = button["pressedBrushId"].get<std::string>();
			const auto buttonBorderBrushId = button["borderBrushId"].get<std::string>();
			const auto buttonTextBrushId = button["textBrushId"].get<std::string>();
			const auto buttonTextFormatId = button["textFormatId"].get<std::string>();

			buttons.at(buttonId)->Initialize(brushes.at(buttonBrushId).Get(), brushes.at(pressedButtonBrushId).Get(), brushes.at(buttonBorderBrushId).Get(), brushes.at(buttonTextBrushId).Get(), textFormats.at(buttonTextFormatId).Get());
		}
	}
}

void ForayClient::InitializeEditorToolButtons()
{
	editorToolButtons.at("addTile")->Initialize(brushes.at("blue").Get(), brushes.at("darkBlue").Get(), brushes.at("gray").Get(), brushes.at("black").Get(), textFormats.at("button").Get());
	editorToolButtons.at("removeTile")->Initialize(brushes.at("blue").Get(), brushes.at("darkBlue").Get(), brushes.at("gray").Get(), brushes.at("black").Get(), textFormats.at("button").Get());
}

void ForayClient::OnWindowSizeChanged(const int width, const int height)
{
	if (!deviceResources->WindowSizeChanged(width, height))
		return;

	g_clientWidth = static_cast<float>(width);
	g_clientHeight = static_cast<float>(height);

	CreateWindowSizeDependentResources();
}

void ForayClient::OnWindowMoved()
{
	const auto r = deviceResources->GetOutputSize();
	deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void ForayClient::OnDeviceLost()
{
	// TODO: Add Direct3D resource cleanup here.
	// may not be needed - classes should be designed with RAII in mind, and clean up
	// after themselves. not sure if there is an exception to this with D3D,
	// so i'm leaving this stubbed out for now.
}

void ForayClient::OnDeviceRestored()
{
	CreateDeviceDependentResources();

	CreateWindowSizeDependentResources();

	SetActiveLayer(activeLayer);
}
