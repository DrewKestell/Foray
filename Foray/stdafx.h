// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

// Windows Header Files
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#include <Windows.h>
#include <windowsx.h>
#include <Xinput.h>
#include <cfloat>
#include <functional>
#include <queue>
#include <list>
#include <memory>
#include <io.h>
#include <fcntl.h>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <assert.h>
#include <stdint.h>
#include <vcruntime_exception.h>
#include <d3d11_1.h>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <dxgi1_6.h>
#include <d3d11.h>
#include <d2d1_3.h>
#include <dwrite_3.h>
#include <xaudio2.h>
#include <fstream>
#include <iostream>
#include <string>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

#include "Graphics/DDSTextureLoader.h"
#include "Extensions.h"
#include "Utility.h"

#include "Include/nlohmann/json.hpp"
using json = nlohmann::json;

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

