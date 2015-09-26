// 005_EnumSupportFormat.cpp
#include <Windows.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include <utility>
#include <cstdlib>
#include <d3d11_3.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
#include "UsingConsole.h"

#pragma comment(lib, "d3d11.lib")

using namespace std;
using namespace Microsoft::WRL;

DWORD PrintSystemError(DWORD error = GetLastError());

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR lpCmdLine, int nCmdShow)
{
    UsingConsole uc;

    HRESULT hr = S_OK;

    // D3D機能レベルの一覧を用意
    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_9_1;
    UINT deviceCreationFlag = 0;

    // D3D11デバイスおよびイメディエイトコンテキストの生成
    ComPtr<ID3D11Device> pDevice;
    ComPtr<ID3D11DeviceContext> pImmediateContext;
    hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        deviceCreationFlag,
        featureLevels,
        _countof(featureLevels),
        D3D11_SDK_VERSION,
        pDevice.GetAddressOf(),
        &featureLevel,
        pImmediateContext.GetAddressOf()
    );
    if(FAILED(hr))
    {
        PrintSystemError(hr);
        return false; 
    }

#define MAKE_FORMAT_PAIR(value) { DXGI_FORMAT_ ## value, #value }
    pair<DXGI_FORMAT, const char *> formats[] =
    {
        MAKE_FORMAT_PAIR(R32G32B32A32_TYPELESS),
        MAKE_FORMAT_PAIR(R32G32B32A32_FLOAT),
        MAKE_FORMAT_PAIR(R32G32B32A32_UINT),
        MAKE_FORMAT_PAIR(R32G32B32A32_SINT),
        MAKE_FORMAT_PAIR(R32G32B32_TYPELESS),
        MAKE_FORMAT_PAIR(R32G32B32_FLOAT),
        MAKE_FORMAT_PAIR(R32G32B32_UINT),
        MAKE_FORMAT_PAIR(R32G32B32_SINT),
        MAKE_FORMAT_PAIR(R16G16B16A16_TYPELESS),
        MAKE_FORMAT_PAIR(R16G16B16A16_FLOAT),
        MAKE_FORMAT_PAIR(R16G16B16A16_UNORM),
        MAKE_FORMAT_PAIR(R16G16B16A16_UINT),
        MAKE_FORMAT_PAIR(R16G16B16A16_SNORM),
        MAKE_FORMAT_PAIR(R16G16B16A16_SINT),
        MAKE_FORMAT_PAIR(R32G32_TYPELESS),
        MAKE_FORMAT_PAIR(R32G32_FLOAT),
        MAKE_FORMAT_PAIR(R32G32_UINT),
        MAKE_FORMAT_PAIR(R32G32_SINT),
        MAKE_FORMAT_PAIR(R32G8X24_TYPELESS),
        MAKE_FORMAT_PAIR(D32_FLOAT_S8X24_UINT),
        MAKE_FORMAT_PAIR(R32_FLOAT_X8X24_TYPELESS),
        MAKE_FORMAT_PAIR(X32_TYPELESS_G8X24_UINT),
        MAKE_FORMAT_PAIR(R10G10B10A2_TYPELESS),
        MAKE_FORMAT_PAIR(R10G10B10A2_UNORM),
        MAKE_FORMAT_PAIR(R10G10B10A2_UINT),
        MAKE_FORMAT_PAIR(R11G11B10_FLOAT),
        MAKE_FORMAT_PAIR(R8G8B8A8_TYPELESS),
        MAKE_FORMAT_PAIR(R8G8B8A8_UNORM),
        MAKE_FORMAT_PAIR(R8G8B8A8_UNORM_SRGB),
        MAKE_FORMAT_PAIR(R8G8B8A8_UINT),
        MAKE_FORMAT_PAIR(R8G8B8A8_SNORM),
        MAKE_FORMAT_PAIR(R8G8B8A8_SINT),
        MAKE_FORMAT_PAIR(R32_TYPELESS),
        MAKE_FORMAT_PAIR(D32_FLOAT),
        MAKE_FORMAT_PAIR(R32_FLOAT),
        MAKE_FORMAT_PAIR(R32_UINT),
        MAKE_FORMAT_PAIR(R32_SINT),
        MAKE_FORMAT_PAIR(R24G8_TYPELESS),
        MAKE_FORMAT_PAIR(D24_UNORM_S8_UINT),
        MAKE_FORMAT_PAIR(R24_UNORM_X8_TYPELESS),
        MAKE_FORMAT_PAIR(X24_TYPELESS_G8_UINT),
        MAKE_FORMAT_PAIR(R8G8_TYPELESS),
        MAKE_FORMAT_PAIR(R8G8_UNORM),
        MAKE_FORMAT_PAIR(R8G8_UINT),
        MAKE_FORMAT_PAIR(R8G8_SNORM),
        MAKE_FORMAT_PAIR(R8G8_SINT),
        MAKE_FORMAT_PAIR(R16_TYPELESS),
        MAKE_FORMAT_PAIR(R16_FLOAT),
        MAKE_FORMAT_PAIR(D16_UNORM),
        MAKE_FORMAT_PAIR(R16_UNORM),
        MAKE_FORMAT_PAIR(R16_UINT),
        MAKE_FORMAT_PAIR(R16_SNORM),
        MAKE_FORMAT_PAIR(R16_SINT),
        MAKE_FORMAT_PAIR(R8_TYPELESS),
        MAKE_FORMAT_PAIR(R8_UNORM),
        MAKE_FORMAT_PAIR(R8_UINT),
        MAKE_FORMAT_PAIR(R8_SNORM),
        MAKE_FORMAT_PAIR(R8_SINT),
        MAKE_FORMAT_PAIR(A8_UNORM),
        MAKE_FORMAT_PAIR(R1_UNORM),
        MAKE_FORMAT_PAIR(R9G9B9E5_SHAREDEXP),
        MAKE_FORMAT_PAIR(R8G8_B8G8_UNORM),
        MAKE_FORMAT_PAIR(G8R8_G8B8_UNORM),
        MAKE_FORMAT_PAIR(BC1_TYPELESS),
        MAKE_FORMAT_PAIR(BC1_UNORM),
        MAKE_FORMAT_PAIR(BC1_UNORM_SRGB),
        MAKE_FORMAT_PAIR(BC2_TYPELESS),
        MAKE_FORMAT_PAIR(BC2_UNORM),
        MAKE_FORMAT_PAIR(BC2_UNORM_SRGB),
        MAKE_FORMAT_PAIR(BC3_TYPELESS),
        MAKE_FORMAT_PAIR(BC3_UNORM),
        MAKE_FORMAT_PAIR(BC3_UNORM_SRGB),
        MAKE_FORMAT_PAIR(BC4_TYPELESS),
        MAKE_FORMAT_PAIR(BC4_UNORM),
        MAKE_FORMAT_PAIR(BC4_SNORM),
        MAKE_FORMAT_PAIR(BC5_TYPELESS),
        MAKE_FORMAT_PAIR(BC5_UNORM),
        MAKE_FORMAT_PAIR(BC5_SNORM),
        MAKE_FORMAT_PAIR(B5G6R5_UNORM),
        MAKE_FORMAT_PAIR(B5G5R5A1_UNORM),
        MAKE_FORMAT_PAIR(B8G8R8A8_UNORM),
        MAKE_FORMAT_PAIR(B8G8R8X8_UNORM),
        MAKE_FORMAT_PAIR(R10G10B10_XR_BIAS_A2_UNORM),
        MAKE_FORMAT_PAIR(B8G8R8A8_TYPELESS),
        MAKE_FORMAT_PAIR(B8G8R8A8_UNORM_SRGB),
        MAKE_FORMAT_PAIR(B8G8R8X8_TYPELESS),
        MAKE_FORMAT_PAIR(B8G8R8X8_UNORM_SRGB),
        MAKE_FORMAT_PAIR(BC6H_TYPELESS),
        MAKE_FORMAT_PAIR(BC6H_UF16),
        MAKE_FORMAT_PAIR(BC6H_SF16),
        MAKE_FORMAT_PAIR(BC7_TYPELESS),
        MAKE_FORMAT_PAIR(BC7_UNORM),
        MAKE_FORMAT_PAIR(BC7_UNORM_SRGB),
        MAKE_FORMAT_PAIR(AYUV),
        MAKE_FORMAT_PAIR(Y410),
        MAKE_FORMAT_PAIR(Y416),
        MAKE_FORMAT_PAIR(NV12),
        MAKE_FORMAT_PAIR(P010),
        MAKE_FORMAT_PAIR(P016),
        MAKE_FORMAT_PAIR(420_OPAQUE),
        MAKE_FORMAT_PAIR(YUY2),
        MAKE_FORMAT_PAIR(Y210),
        MAKE_FORMAT_PAIR(Y216),
        MAKE_FORMAT_PAIR(NV11),
        MAKE_FORMAT_PAIR(AI44),
        MAKE_FORMAT_PAIR(IA44),
        MAKE_FORMAT_PAIR(P8),
        MAKE_FORMAT_PAIR(A8P8),
        MAKE_FORMAT_PAIR(B4G4R4A4_UNORM),
        MAKE_FORMAT_PAIR(P208),
        MAKE_FORMAT_PAIR(V208),
        MAKE_FORMAT_PAIR(V408),
        MAKE_FORMAT_PAIR(FORCE_UINT),
    };
#undef MAKE_FORMAT_PAIR

#define MAKE_SUPPORT_PAIR(value) { D3D11_FORMAT_SUPPORT_ ## value, #value }
    pair<D3D11_FORMAT_SUPPORT, const char *> support[] = {
        MAKE_SUPPORT_PAIR(BUFFER),
        MAKE_SUPPORT_PAIR(IA_VERTEX_BUFFER),
        MAKE_SUPPORT_PAIR(IA_INDEX_BUFFER),
        MAKE_SUPPORT_PAIR(SO_BUFFER),
        MAKE_SUPPORT_PAIR(TEXTURE1D),
        MAKE_SUPPORT_PAIR(TEXTURE2D),
        MAKE_SUPPORT_PAIR(TEXTURE3D),
        MAKE_SUPPORT_PAIR(TEXTURECUBE),
        MAKE_SUPPORT_PAIR(SHADER_LOAD),
        MAKE_SUPPORT_PAIR(SHADER_SAMPLE),
        MAKE_SUPPORT_PAIR(SHADER_SAMPLE_COMPARISON),
        MAKE_SUPPORT_PAIR(SHADER_SAMPLE_MONO_TEXT),
        MAKE_SUPPORT_PAIR(MIP),
        MAKE_SUPPORT_PAIR(MIP_AUTOGEN),
        MAKE_SUPPORT_PAIR(RENDER_TARGET),
        MAKE_SUPPORT_PAIR(BLENDABLE),
        MAKE_SUPPORT_PAIR(DEPTH_STENCIL),
        MAKE_SUPPORT_PAIR(CPU_LOCKABLE),
        MAKE_SUPPORT_PAIR(MULTISAMPLE_RESOLVE),
        MAKE_SUPPORT_PAIR(DISPLAY),
        MAKE_SUPPORT_PAIR(CAST_WITHIN_BIT_LAYOUT),
        MAKE_SUPPORT_PAIR(MULTISAMPLE_RENDERTARGET),
        MAKE_SUPPORT_PAIR(MULTISAMPLE_LOAD),
        MAKE_SUPPORT_PAIR(SHADER_GATHER),
        MAKE_SUPPORT_PAIR(BACK_BUFFER_CAST),
        MAKE_SUPPORT_PAIR(TYPED_UNORDERED_ACCESS_VIEW),
        MAKE_SUPPORT_PAIR(SHADER_GATHER_COMPARISON),
        MAKE_SUPPORT_PAIR(DECODER_OUTPUT),
        MAKE_SUPPORT_PAIR(VIDEO_PROCESSOR_OUTPUT),
        MAKE_SUPPORT_PAIR(VIDEO_PROCESSOR_INPUT),
        MAKE_SUPPORT_PAIR(VIDEO_ENCODER),
    };
#undef MAKE_SUPPORT_PAIR

    ofstream fout("SupportedFormat.csv");
    for(auto & s : support)
    {
        fout << "," << s.second;
    }
    fout << endl;

    for(auto & format : formats)
    {
        UINT supported;
        hr = pDevice->CheckFormatSupport(format.first, &supported);
        if(FAILED(hr))
        {
            continue;
        }

        fout << format.second;

        for(auto & s : support)
        {
            if(supported & s.first)
            {
                fout << ",O";
            }
            else
            {
                fout << ",";
            }
        }
        fout << endl;
    }

    fout.close();

    return 0;
}

DWORD PrintSystemError(DWORD error)
{
    WCHAR * pBuffer = nullptr;
    DWORD length = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPWSTR>(&pBuffer),
        0,
        nullptr
    );

    if(length > 0 && pBuffer != nullptr)
    {
        wcout << pBuffer << endl;
        LocalFree(pBuffer);
    }

    return error;
}
