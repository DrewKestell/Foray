#include "stdafx.h"
#include <Windows.h>
#include <windowsx.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <memory>
#include "Game.h"
#include "Constants.h"
#include "Events/Event.h"
#include "Events/MouseEvent.h"
#include "Events/EventHandler.h"
#include "Events/SystemKeyDownEvent.h"

// Global Variables:
HINSTANCE hInst;
wchar_t szWindowClass[] = L"win32app";
wchar_t szTitle[] = L"Foray Client";

static EventHandler eventHandler;
static auto gamepad = std::make_unique<Gamepad>(0);

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void HandleGamepadInput();

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	if (!DirectX::XMVerifyCPUSupport())
		return 1;

	const HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
	if (FAILED(hr))
		return 1;

	static auto game = std::make_unique<Game>(eventHandler);

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
	wcex.lpszClassName = szWindowClass;
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
		szWindowClass,
		szTitle,
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

	// Wrap the WindowPtr in our Game
	SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(game.get()));

	GetClientRect(hWnd, &rc);
	game->Initialize(hWnd, rc.right - rc.left, rc.bottom - rc.top);

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
			game->Tick();
		}
	}

	game.reset();
	gamepad.reset();

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

	auto game = reinterpret_cast<Game*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

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
		eventHandler.QueueEvent(e);
		break;

	case WM_MOUSEMOVE:
		e = std::make_unique<MouseEvent>(EventType::MouseMove, (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam));
		eventHandler.QueueEvent(e);

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
			switch (wParam)
			{
			case VK_MENU:
				const auto key = MapLeftRightKeys(wParam, lParam);
				e = std::make_unique<SystemKeyDownEvent>(key);
				eventHandler.QueueEvent(e);
				break;
			}
		}

		break;

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
			// TODO
		}

		if (gamepadState.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
		{
			// TODO
		}

		if (gamepadState.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
		{
			// TODO
		}

		if (gamepadState.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
		{
			// TODO
		}

		if (gamepadState.wButtons & XINPUT_GAMEPAD_A)
		{
			// TODO
		}

		if (gamepadState.wButtons & XINPUT_GAMEPAD_B)
		{
			// TODO
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