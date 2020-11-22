#include "stdafx.h"
#include "Gamepad.h"
#include "ForayClient.h"
#include "Constants.h"
#include "ObjectManager.h"
#include "Events/MouseEvent.h"
#include "Events/EventHandler.h"
#include "Events/KeyDownEvent.h"
#include "Events/KeyUpEvent.h"
#include "Events/GamepadInputEvent.h"
#include "Events/ChangeActiveLayerEvent.h"

// outgoing globals
std::unique_ptr<EventHandler> g_eventHandler;

// local variables
static HINSTANCE hInst;
static wchar_t windowClass[] = L"win32app";
static std::unique_ptr<Gamepad> gamepad;
static std::unique_ptr<ForayClient> forayClient;

// TODO: move this into its own class
static bool upPressed = false;
static bool downPressed = false;
static bool leftPressed = false;
static bool rightPressed = false;
static bool aPressed = false;
static bool bPressed = false;
static bool rTriggerPressed = false;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void HandleGamepadInput();
void AllocateConsole();

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	AllocateConsole();

	// Ensure CPU supports DirecXMath constructs
	if (!XMVerifyCPUSupport())
		return 1;

	// Initialize COM multithreaded support
	if (FAILED(CoInitializeEx(nullptr, COINITBASE_MULTITHREADED)))
		return 1;

	// Register class
	WNDCLASSEX wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.hIcon = (HICON)LoadImage( // returns a HANDLE so we have to cast to HICON
		NULL,             // hInstance must be NULL when loading from a file
		L"Foray.ico",     // the icon file name
		IMAGE_ICON,       // specifies that the file is an icon
		0,                // width of the image (we'll specify default later on)
		0,                // height of the image
		LR_LOADFROMFILE | // we want to load a file (as opposed to a resource)
		LR_DEFAULTSIZE |  // default metrics based on the type (IMAGE_ICON, 32x32)
		LR_SHARED         // let the system release the handle when it's no longer used
	);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = windowClass;
	wcex.hIconSm = NULL;  // will automatically use the value of hIcon when we set this to null
	if (!RegisterClassEx(&wcex))
		return 1;

	// Create window
	RECT rc = { 0, 0, long{ static_cast<int>(CLIENT_WIDTH) }, long{ static_cast<int>(CLIENT_HEIGHT) } };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	// TODO: Change to CreateWindowExW(WS_EX_TOPMOST, L"Direct3D_Win32_Game1WindowClass", L"Direct3D Win32 Game1", WS_POPUP,
	// to default to fullscreen.
	HWND hWnd = CreateWindowExW(
		0,
		windowClass,
		L"Foray Client",
		WS_OVERLAPPEDWINDOW,
		0,
		0,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr
	);
	if (!hWnd)
		return 1;

	// TODO: Change nCmdShow to SW_SHOWMAXIMIZED to default to fullscreen.
	ShowWindow(hWnd, nCmdShow);

	// TODO: why is this necessary? everything works fine without it.
	SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(forayClient.get()));

	// initialize all dependencies
	g_eventHandler = std::make_unique<EventHandler>();
	gamepad = std::make_unique<Gamepad>(0);
	GetClientRect(hWnd, &rc);
	forayClient = std::make_unique<ForayClient>(hWnd, rc.right - rc.left, rc.bottom - rc.top);

	std::unique_ptr<Event> e = std::make_unique<ChangeActiveLayerEvent>(Layer::Game);
	g_eventHandler->QueueEvent(e);

	// Main message loop
	MSG msg{ nullptr };
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			HandleGamepadInput();
			forayClient->Tick();
		}
	}

	CoUninitialize();

	return (int)msg.wParam;
}

WPARAM MapLeftRightKeys(WPARAM vk, LPARAM lParam)
{
	const UINT scancode = (lParam & 0x00ff0000) >> 16;
	const int extended = (lParam & 0x01000000) != 0;

	switch (vk)
	{
	case VK_SHIFT:
		if (MapVirtualKey(scancode, MAPVK_VSC_TO_VK_EX) == VK_LSHIFT)
			return VK_LSHIFT;
		else
			return VK_RSHIFT;
		break;
	case VK_CONTROL:
		if (extended)
			return VK_RCONTROL;
		else
			return VK_LCONTROL;
		break;
	case VK_MENU:
		if (extended)
			return VK_RMENU;
		else
			return VK_LMENU;
		break;
	}

	throw std::exception("Something went wrong.");
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	static bool inSizemove = false;
	static bool inFullscreen = false;

	auto game = reinterpret_cast<ForayClient*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	WPARAM keyCode{ 0 };
	std::unique_ptr<Event> e;
	switch (message)
	{
	case WM_PAINT:
		if (inSizemove && game)
		{
			game->Tick();
		}
		else
		{
			hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
		}
		break;

	case WM_MOVE:
		if (game)
		{
			game->OnWindowMoved();
		}
		break;

	case WM_SIZE:
		if (game)
		{
			game->OnWindowSizeChanged(LOWORD(lParam), HIWORD(lParam));
		}

		break;

	case WM_ENTERSIZEMOVE:
		inSizemove = true;
		break;

	case WM_EXITSIZEMOVE:
		inSizemove = false;
		if (game)
		{
			RECT rc;
			GetClientRect(hWnd, &rc);

			game->OnWindowSizeChanged(rc.right - rc.left, rc.bottom - rc.top);
		}
		break;

	case WM_GETMINMAXINFO:
		{
			auto info = reinterpret_cast<MINMAXINFO*>(lParam);
			info->ptMinTrackSize.x = 320;
			info->ptMinTrackSize.y = 200;
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_LBUTTONDOWN:
		e = std::make_unique<MouseEvent>(EventType::LeftMouseDown, (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam));
		g_eventHandler->QueueEvent(e);
		break;
	case WM_MBUTTONDOWN:
		e = std::make_unique<MouseEvent>(EventType::MiddleMouseDown, (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam));
		g_eventHandler->QueueEvent(e);
		break;
	case WM_RBUTTONDOWN:
		e = std::make_unique<MouseEvent>(EventType::RightMouseDown, (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam));
		g_eventHandler->QueueEvent(e);
		break;
	case WM_LBUTTONUP:
		e = std::make_unique<MouseEvent>(EventType::LeftMouseUp, (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam));
		g_eventHandler->QueueEvent(e);
		break;
	case WM_MBUTTONUP:
		e = std::make_unique<MouseEvent>(EventType::MiddleMouseUp, (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam));
		g_eventHandler->QueueEvent(e);
		g_eventHandler;
	case WM_RBUTTONUP:
		e = std::make_unique<MouseEvent>(EventType::RightMouseUp, (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam));
		g_eventHandler->QueueEvent(e);
		break;
	case WM_MOUSEMOVE:
		e = std::make_unique<MouseEvent>(EventType::MouseMove, (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam));
		g_eventHandler->QueueEvent(e);

		if (inSizemove)
		{
			RECT rc;
			GetClientRect(hWnd, &rc);

			game->OnWindowSizeChanged(rc.right - rc.left, rc.bottom - rc.top);
		}
		break;

	case WM_SYSKEYDOWN:
		if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
		{
			// Implements the classic ALT+ENTER fullscreen toggle
			if (inFullscreen)
			{
				SetWindowLongPtr(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
				SetWindowLongPtr(hWnd, GWL_EXSTYLE, 0);

				ShowWindow(hWnd, SW_SHOWNORMAL);

				SetWindowPos(hWnd, nullptr, 0, 0, static_cast<int>(CLIENT_WIDTH), static_cast<int>(CLIENT_HEIGHT), SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
			}
			else
			{
				SetWindowLongPtr(hWnd, GWL_STYLE, 0);
				SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);

				SetWindowPos(hWnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

				ShowWindow(hWnd, SW_SHOWMAXIMIZED);
			}

			inFullscreen = !inFullscreen;
		}
		else
		{
			const auto key = MapLeftRightKeys(wParam, lParam);
			e = std::make_unique<KeyDownEvent>(key);
			g_eventHandler->QueueEvent(e);
		}

		break;

	case WM_SYSKEYUP:
	{
		const auto key = MapLeftRightKeys(wParam, lParam);
		e = std::make_unique<KeyUpEvent>(key);
		g_eventHandler->QueueEvent(e);
		break;
	}

	case WM_KEYDOWN:
		switch (wParam)
		{
			case VK_SHIFT:
			case VK_CONTROL:
				keyCode = MapLeftRightKeys(wParam, lParam);
				break;
			default:
				keyCode = wParam;
				break;
		}
		e = std::make_unique<KeyDownEvent>(keyCode);
		g_eventHandler->QueueEvent(e);
		break;

	case WM_KEYUP:
		switch (wParam)
		{
			case VK_SHIFT:
			case VK_CONTROL:
				keyCode = MapLeftRightKeys(wParam, lParam);
				break;
			default:
				keyCode = wParam;
				break;
		}
		e = std::make_unique<KeyUpEvent>(keyCode);
		g_eventHandler->QueueEvent(e);
		break;

	case WM_CHAR:
		switch (wParam)
		{
			case 0x08: // Ignore backspace.
				break;
			case 0x0A: // Ignore linefeed.   
				break;
			case 0x1B: // Ignore escape. 
				break;
			case 0x09: // Ignore tab.          
				break;
			case 0x0D: // Ignore carriage return.
				break;
			default:   // Process a normal character press.            
				auto ch = static_cast<wchar_t>(wParam);
				e = std::make_unique<KeyDownEvent>(ch);
				g_eventHandler->QueueEvent(e);
				break;
		}
	case WM_MENUCHAR:
		// A menu is active and the user presses a key that does not correspond
		// to any mnemonic or accelerator key. Ignore so we don't produce an error beep.
		// Primarily useful for avoiding the beep on ALT + ENTER.
		return MAKELRESULT(0, MNC_CLOSE);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

	return 0;
}

// Documentation: https://docs.microsoft.com/en-us/windows/win32/xinput/getting-started-with-xinput
void HandleGamepadInput()
{
	if (gamepad->IsConnected())
	{
		const auto gamepadState = gamepad->GetState().Gamepad;

		if (gamepadState.wButtons & XINPUT_GAMEPAD_DPAD_UP)
		{
			if (!upPressed)
			{
				std::unique_ptr<Event> e = std::make_unique<GamepadInputEvent>(EventType::GamepadInput, XINPUT_GAMEPAD_DPAD_UP, 0, true);
				g_eventHandler->QueueEvent(e);
			}
			upPressed = true;
		}
		else
		{
			if (upPressed)
			{
				std::unique_ptr<Event> e = std::make_unique<GamepadInputEvent>(EventType::GamepadInput, XINPUT_GAMEPAD_DPAD_UP, 0, false);
				g_eventHandler->QueueEvent(e);
			}
			upPressed = false;
		}

		if (gamepadState.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
		{
			if (!downPressed)
			{
				std::unique_ptr<Event> e = std::make_unique<GamepadInputEvent>(EventType::GamepadInput, XINPUT_GAMEPAD_DPAD_DOWN, 0, true);
				g_eventHandler->QueueEvent(e);
			}
			downPressed = true;
		}
		else
		{
			if (downPressed)
			{
				std::unique_ptr<Event> e = std::make_unique<GamepadInputEvent>(EventType::GamepadInput, XINPUT_GAMEPAD_DPAD_DOWN, 0, false);
				g_eventHandler->QueueEvent(e);
			}
			downPressed = false;
		}

		if (gamepadState.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
		{
			if (!leftPressed)
			{
				std::unique_ptr<Event> e = std::make_unique<GamepadInputEvent>(EventType::GamepadInput, XINPUT_GAMEPAD_DPAD_LEFT, 0, true);
				g_eventHandler->QueueEvent(e);
			}
			leftPressed = true;
		}
		else
		{
			if (leftPressed)
			{
				std::unique_ptr<Event> e = std::make_unique<GamepadInputEvent>(EventType::GamepadInput, XINPUT_GAMEPAD_DPAD_LEFT, 0, false);
				g_eventHandler->QueueEvent(e);
			}
			leftPressed = false;
		}

		if (gamepadState.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
		{
			if (!rightPressed)
			{
				std::unique_ptr<Event> e = std::make_unique<GamepadInputEvent>(EventType::GamepadInput, XINPUT_GAMEPAD_DPAD_RIGHT, 0, true);
				g_eventHandler->QueueEvent(e);
			}
			rightPressed = true;
		}
		else
		{
			if (rightPressed)
			{
				std::unique_ptr<Event> e = std::make_unique<GamepadInputEvent>(EventType::GamepadInput, XINPUT_GAMEPAD_DPAD_RIGHT, 0, false);
				g_eventHandler->QueueEvent(e);
			}
			rightPressed = false;
		}

		if (gamepadState.wButtons & XINPUT_GAMEPAD_A)
		{
			if (!aPressed)
			{
				std::unique_ptr<Event> e = std::make_unique<GamepadInputEvent>(EventType::GamepadInput, XINPUT_GAMEPAD_A, 0, true);
				g_eventHandler->QueueEvent(e);
			}
			aPressed = true;
		}
		else
		{
			if (aPressed)
			{
				std::unique_ptr<Event> e = std::make_unique<GamepadInputEvent>(EventType::GamepadInput, XINPUT_GAMEPAD_A, 0, false);
				g_eventHandler->QueueEvent(e);
			}
			aPressed = false;
		}

		if (gamepadState.wButtons & XINPUT_GAMEPAD_B)
		{
			if (!bPressed)
			{
				std::unique_ptr<Event> e = std::make_unique<GamepadInputEvent>(EventType::GamepadInput, XINPUT_GAMEPAD_B, 0, true);
				g_eventHandler->QueueEvent(e);
			}
			bPressed = true;
		}
		else
		{
			if (bPressed)
			{
				std::unique_ptr<Event> e = std::make_unique<GamepadInputEvent>(EventType::GamepadInput, XINPUT_GAMEPAD_B, 0, false);
				g_eventHandler->QueueEvent(e);
			}
			bPressed = false;
		}

		if (gamepadState.bRightTrigger > 0)
		{
			if (!rTriggerPressed)
			{
				std::unique_ptr<Event> e = std::make_unique<GamepadInputEvent>(EventType::GamepadInput, VK_PAD_RTRIGGER, 0, true);
				g_eventHandler->QueueEvent(e);
			}
			rTriggerPressed = true;
		}
		else
		{
			if (rTriggerPressed)
			{
				std::unique_ptr<Event> e = std::make_unique<GamepadInputEvent>(EventType::GamepadInput, VK_PAD_RTRIGGER, 0, false);
				g_eventHandler->QueueEvent(e);
			}
			rTriggerPressed = false;
		}

		if (gamepadState.wButtons & XINPUT_GAMEPAD_X)
		{
			// TODO
		}

		if (gamepadState.wButtons & XINPUT_GAMEPAD_START)
		{
			// TODO
		}

		if (gamepadState.wButtons & XINPUT_GAMEPAD_BACK)
		{
			// TODO
		}

		if (gamepadState.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)
		{
			// TODO
		}

		if (gamepadState.wButtons & XINPUT_GAMEPAD_LEFT_THUMB)
		{
			// TODO
		}

		if (gamepadState.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
		{
			// TODO
		}

		if (gamepadState.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)
		{
			// TODO
		}

		// Trigger values fall between 0-255
		// XINPUT_GAMEPAD_TRIGGER_THRESHOLD can be used to filter trigger input

		const auto leftTriggerValue = gamepadState.bLeftTrigger;
		if (leftTriggerValue > 0)
		{
			// TODO
		}

		const auto rightTriggerValue = gamepadState.bRightTrigger;
		if (rightTriggerValue > 0)
		{
			// TODO
		}

		// Analog Stick values are a signed value between -32768 and 32767 describing the position
		//   of the thumbstick. A value of 0 is centered. Negative values signify down or to the left.
		//   Positive values signify up or to the right. The constants XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE
		//   or XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE can be used as a positive and negative value to filter 
		//   a thumbstick input.

		const auto leftThumbstickX = gamepadState.sThumbLX;
		// TODO

		const auto leftThumbstickY = gamepadState.sThumbLY;
		// TODO

		const auto rightThumbstickX = gamepadState.sThumbRX;
		// TODO

		const auto rightThumbstickY = gamepadState.sThumbRY;
		// TODO
	}
}

void AllocateConsole()
{
	AllocConsole();
	HANDLE stdHandle;
	int hConsole;
	FILE* fp;
	stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	hConsole = _open_osfhandle((long)stdHandle, _O_TEXT);
	fp = _fdopen(hConsole, "w");
	freopen_s(&fp, "CONOUT$", "w", stdout);
}