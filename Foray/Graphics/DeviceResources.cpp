#include "../stdafx.h"
#include "DeviceResources.h"

DeviceResources::DeviceResources() noexcept
{
}

// Create and configure D3DDevice and D2DDevice, and all associated
//   resources that aren't dependent on the size of the window.
void DeviceResources::CreateDeviceResources()
{
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	CreateFactory();

	static const D3D_FEATURE_LEVEL s_featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;

	HRESULT hr;
	hr = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		0,
		creationFlags,
		s_featureLevels,
		1,
		D3D11_SDK_VERSION,
		device.GetAddressOf(),  // Returns the Direct3D device created.
		&d3dFeatureLevel,       // Returns feature level of device created.
		context.GetAddressOf()  // Returns the device immediate context.
	);
	if (FAILED(hr))
		throw std::exception("Failed to create device.");

#ifndef NDEBUG
	ComPtr<ID3D11Debug> d3dDebug;
	if (SUCCEEDED(device.As(&d3dDebug)))
	{
		ComPtr<ID3D11InfoQueue> d3dInfoQueue;
		if (SUCCEEDED(d3dDebug.As(&d3dInfoQueue)))
		{
#ifdef _DEBUG
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif
			D3D11_MESSAGE_ID hide[] =
			{
				D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
			};
			D3D11_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = _countof(hide);
			filter.DenyList.pIDList = hide;
			d3dInfoQueue->AddStorageFilterEntries(&filter);
		}
	}
#endif

	hr = device.As(&d3dDevice);
	if (FAILED(hr))
		throw std::exception("Failed to query interface.");

	hr = context.As(&d3dContext);
	if (FAILED(hr))
		throw std::exception("Failed to query interface.");

	// WriteFactory is used for Text related tasks, such as CreateTextLayout.
	// Text rendering happens through the D2DContext, for example:
	//   writeFactory->CreateTextLayout...
	//   d2dContext->DrawTextLayout...
	hr = DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		&writeFactory
	);
	if (FAILED(hr))
		throw std::exception("Failed to create DirectWrite Factory.");

	// D2DFactory is used to create 2D geometry, for example:
	//   d2dFactory->CreateRoundedRectangleGeometry...
	//   d2dContext->DrawGeometry...
	D2D1_FACTORY_OPTIONS options;
#ifdef _DEBUG
	options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif
#ifndef _DEBUG
	options.debugLevel = D2D1_DEBUG_LEVEL_NONE;
#endif
	hr = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_MULTI_THREADED,
		__uuidof(ID2D1Factory),
		&options,
		&d2dFactory
	);
	if (FAILED(hr))
		throw std::exception("Failed to create D2D1 Factory.");

	// Get a reference to the DXGIDevice to use in creation of the D2DDevice
	ComPtr<IDXGIDevice> dxgiDevice;
	hr = device.As(&dxgiDevice);
	if (FAILED(hr))
		throw std::exception("Failed to query DXGIDevice.");

	// Create D2DDevice
	hr = d2dFactory->CreateDevice(dxgiDevice.Get(), d2dDevice.GetAddressOf());
	if (FAILED(hr))
		throw std::exception("Failed to create D2DDevice.");

	// Create D2DContext
	hr = d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS, d2dContext.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		throw std::exception("Failed to reate D2DDeviceContext.");

	// Get MSAA settings
	hr = device->CheckMultisampleQualityLevels(DXGI_FORMAT_B8G8R8A8_UNORM, msaaCount, &msaaQuality);
	if (FAILED(hr))
		throw std::exception("Failed to get MSAA settings.");

	// create Shaders
	spriteVertexShaderBuffer = LoadShader(L"SpriteVertexShader.cso");
	device->CreateVertexShader(spriteVertexShaderBuffer.buffer, spriteVertexShaderBuffer.size, nullptr, spriteVertexShader.ReleaseAndGetAddressOf());

	spritePixelShaderBuffer = LoadShader(L"SpritePixelShader.cso");
	device->CreatePixelShader(spritePixelShaderBuffer.buffer, spritePixelShaderBuffer.size, nullptr, spritePixelShader.ReleaseAndGetAddressOf());
}

void DeviceResources::CreateFactory()
{
#if defined(_DEBUG) && (_WIN32_WINNT >= 0x0603 /*_WIN32_WINNT_WINBLUE*/)
	bool debugDXGI = false;
	{
		ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
		{
			debugDXGI = true;

			const auto hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(dxgiFactory.ReleaseAndGetAddressOf()));
			if (FAILED(hr))
				throw std::exception("Failed to create DXGIFactory.");

			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

			DXGI_INFO_QUEUE_MESSAGE_ID hide[] =
			{
				80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */,
			};
			DXGI_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = _countof(hide);
			filter.DenyList.pIDList = hide;
			dxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
		}
	}

	if (!debugDXGI)
#endif
	{
		const auto hr = CreateDXGIFactory1(IID_PPV_ARGS(dxgiFactory.ReleaseAndGetAddressOf()));
		if (FAILED(hr))
			throw std::exception("Failed to create DXGIFactory.");
	}
}

ShaderBuffer DeviceResources::LoadShader(const std::wstring filename)
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

// These resources need to be recreated every time the window size is changed.
void DeviceResources::CreateWindowSizeDependentResources()
{
	// Clear the previous window size specific context.
	ID3D11RenderTargetView* nullViews[] = { nullptr };
	d3dContext->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);
	d2dContext->SetTarget(nullptr);
	backBufferRenderTargetView.Reset();
	offscreenRenderTargetView.Reset();
	depthStencilView.Reset();
	backBufferRenderTarget.Reset();
	offscreenRenderTarget.Reset();
	depthStencil.Reset();
	d3dContext->Flush();

	// Determine the render target size in pixels.
	const UINT renderTargetWidth = std::max<UINT>(outputSize.right - outputSize.left, 1);
	const UINT renderTargetHeight = std::max<UINT>(outputSize.bottom - outputSize.top, 1);

	if (swapChain)
	{
		// If the swap chain already exists, resize it.
		const HRESULT hr = swapChain->ResizeBuffers(
			backBufferCount,
			renderTargetWidth,
			renderTargetHeight,
			backBufferFormat,
			0
		);

		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
#ifdef _DEBUG
			char buff[64] = {};
			sprintf_s(buff, "Device Lost on ResizeBuffers: Reason code 0x%08X\n", (hr == DXGI_ERROR_DEVICE_REMOVED) ? d3dDevice->GetDeviceRemovedReason() : hr);
			OutputDebugStringA(buff);
#endif
			// If the device was removed for any reason, a new device and swap chain will need to be created.
			HandleDeviceLost();

			// Everything is set up now. Do not continue execution of this method. HandleDeviceLost will reenter this method 
			// and correctly set up the new device.
			return;
		}
		else
		{
			if (FAILED(hr))
				throw new std::exception("Failed to resize buffers");
		}
	}
	else
	{
		// Create a descriptor for the swap chain.
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = renderTargetWidth;
		swapChainDesc.Height = renderTargetHeight;
		swapChainDesc.Format = backBufferFormat;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = backBufferCount;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
		swapChainDesc.Flags = 0;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
		fsSwapChainDesc.Windowed = TRUE;

		// Create a SwapChain from a Win32 window.
		auto hr = dxgiFactory->CreateSwapChainForHwnd(
			d3dDevice.Get(),
			window,
			&swapChainDesc,
			&fsSwapChainDesc,
			nullptr,
			swapChain.ReleaseAndGetAddressOf()
		);
		if (FAILED(hr))
			throw new std::exception("Failed to create swap chain.");

		// This class does not support exclusive full-screen mode and prevents DXGI from responding to the ALT+ENTER shortcut
		hr = dxgiFactory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER);
		if (FAILED(hr))
			throw new std::exception("Failed to make window association.");
	}

	auto hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferRenderTarget);
	if (FAILED(hr))
		throw new std::exception("Failed to get buffer.");

	hr = d3dDevice->CreateRenderTargetView(backBufferRenderTarget.Get(), NULL, &backBufferRenderTargetView);
	if (FAILED(hr))
		throw new std::exception("Failed to create render target view.");

	// Create a depth stencil view for use with 3D rendering.
	CD3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Format = depthBufferFormat;
	depthStencilDesc.Width = renderTargetWidth;
	depthStencilDesc.Height = renderTargetHeight;
	depthStencilDesc.SampleDesc.Count = msaaCount;
	depthStencilDesc.SampleDesc.Quality = msaaQuality - 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	hr = d3dDevice->CreateTexture2D(
		&depthStencilDesc,
		nullptr,
		depthStencil.ReleaseAndGetAddressOf()
	);
	if (FAILED(hr))
		throw new std::exception("Failed to create texture.");

	CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2DMS);
	hr = d3dDevice->CreateDepthStencilView(
		depthStencil.Get(),
		&depthStencilViewDesc,
		depthStencilView.ReleaseAndGetAddressOf()
	);
	if (FAILED(hr))
		throw new std::exception("Failed to create depth stencil view.");

	// Create offscreen render target
	D3D11_TEXTURE2D_DESC surfaceDesc;
	surfaceDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	surfaceDesc.Width = renderTargetWidth;
	surfaceDesc.Height = renderTargetHeight;
	surfaceDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
	surfaceDesc.MipLevels = 1;
	surfaceDesc.ArraySize = 1;
	surfaceDesc.SampleDesc.Count = msaaCount;
	surfaceDesc.SampleDesc.Quality = msaaQuality - 1;
	surfaceDesc.CPUAccessFlags = 0;
	surfaceDesc.MiscFlags = 0;
	surfaceDesc.Usage = D3D11_USAGE_DEFAULT;

	hr = d3dDevice->CreateTexture2D(&surfaceDesc, nullptr, &offscreenRenderTarget);
	if (FAILED(hr))
		throw new std::exception("Failed to create texture.");

	CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc(D3D11_RTV_DIMENSION_TEXTURE2DMS);
	d3dDevice->CreateRenderTargetView(offscreenRenderTarget.Get(), &renderTargetViewDesc, &offscreenRenderTargetView);
	if (FAILED(hr))
		throw new std::exception("Failed to create render target view.");

	// Get reference to DXGISurface and create a bitmap from that surface
	// to use as a rendering target for our D2DContext.
	D2D1_BITMAP_PROPERTIES1 bp;
	bp.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	bp.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
	bp.dpiX = 96.0f;
	bp.dpiY = 96.0f;
	bp.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
	bp.colorContext = nullptr;

	ComPtr<IDXGISurface> dxgiSurface;
	hr = offscreenRenderTarget.As(&dxgiSurface);
	if (FAILED(hr))
		throw new std::exception("Failed to query offscreen render target interface.");

	ComPtr<ID2D1Bitmap1> targetBitmap;
	hr = d2dContext->CreateBitmapFromDxgiSurface(dxgiSurface.Get(), &bp, &targetBitmap);
	if (FAILED(hr))
		throw new std::exception("Failed to create bitmap.");

	d2dContext->SetTarget(targetBitmap.Get());

	// Set the 3D rendering viewport to target the entire window.
	screenViewport = CD3D11_VIEWPORT(
		0.0f,
		0.0f,
		static_cast<float>(renderTargetWidth),
		static_cast<float>(renderTargetHeight)
	);
}

// Recreate all device resources and set them back to the current state.
void DeviceResources::HandleDeviceLost()
{
	if (deviceNotify)
	{
		deviceNotify->OnDeviceLost();
	}

	d2dContext.Reset();
	d2dDevice.Reset();
	d2dFactory.Reset();
	writeFactory.Reset();
	depthStencilView.Reset();
	backBufferRenderTargetView.Reset();
	offscreenRenderTargetView.Reset();
	backBufferRenderTarget.Reset();
	offscreenRenderTarget.Reset();
	depthStencil.Reset();
	swapChain.Reset();
	d3dContext.Reset();

#ifdef _DEBUG
	{
		ComPtr<ID3D11Debug> d3dDebug;
		if (SUCCEEDED(d3dDevice.As(&d3dDebug)))
		{
			d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY);
		}
	}
#endif

	d3dDevice.Reset();
	dxgiFactory.Reset();

	CreateDeviceResources();
	CreateWindowSizeDependentResources();

	if (deviceNotify)
	{
		deviceNotify->OnDeviceRestored();
	}
}

// This method is called when the Win32 window is created (or re-created).
void DeviceResources::SetWindow(HWND window, int width, int height)
{
	this->window = window;

	outputSize.top = 0;
	outputSize.left = 0;
	outputSize.right = width;
	outputSize.bottom = height;
}

void DeviceResources::Present()
{
	HRESULT hr;

	// The first argument instructs DXGI to block until VSync, putting the application
	// to sleep until the next VSync. This ensures we don't waste any cycles rendering
	// frames that will never be displayed to the screen.
	hr = swapChain->Present(0, 0);

	// Discard the contents of the render target.
	// This is a valid operation only when the existing contents will be entirely
	// overwritten. If dirty or scroll rects are used, this call should be removed.
	d3dContext->DiscardView(backBufferRenderTargetView.Get());
	d3dContext->DiscardView(offscreenRenderTargetView.Get());
	d3dContext->DiscardView(depthStencilView.Get());

	// If the device was removed either by a disconnection or a driver upgrade, we 
	// must recreate all device resources.
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
#ifdef _DEBUG
		char buff[64] = {};
		sprintf_s(buff, "Device I AM A GAY MAGICIAN Lost on Present: Reason code 0x%08X\n", (hr == DXGI_ERROR_DEVICE_REMOVED) ? d3dDevice->GetDeviceRemovedReason() : hr);
		OutputDebugStringA(buff);
#endif
		HandleDeviceLost();
	}
	else
	{
		if (FAILED(hr))
			throw new std::exception("Failed to present render target.");

		if (!dxgiFactory->IsCurrent())
		{
			// Output information is cached on the DXGI Factory. If it is stale we need to create a new factory.
			CreateFactory();
		}
	}
}

bool DeviceResources::WindowSizeChanged(int width, int height)
{
	RECT newRc;
	newRc.left = 0;
	newRc.top = 0;
	newRc.right = width;
	newRc.bottom = height;
	if (newRc == outputSize)
	{
		// Handle color space settings for HDR
		//UpdateColorSpace();

		return false;
	}

	outputSize = newRc;
	CreateWindowSizeDependentResources();
	return true;
}