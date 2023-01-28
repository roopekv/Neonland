#pragma once

#define NOMINMAX
#include <Windows.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Composition.h>
#include <unknwn.h>
#include <winrt/Windows.Graphics.Display.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.System.Threading.h>
#include <winrt/Windows.UI.Input.h>

#include <initguid.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <wincodec.h>

#include "d3dx12/d3dx12.h"

#include "WaveFrontReader.h"
#include "DirectXTex.h"

#if defined(DEBUG) || defined(_DEBUG)
#include <d3d12sdklayers.h>
#include <iostream>
#endif

#include <memory>
#include <vector>
#include <array>
#include <map>
#include <string>
#include <fstream>