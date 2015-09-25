// 010_DrawDDSTexture.cpp
#include <Windows.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include <iterator>
#include <cstdlib>
#include <vector>
#include <d3d11_3.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include "UsingConsole.h"

using namespace std;
using namespace Microsoft::WRL;
using namespace DirectX;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool OnCreate(HWND hWnd);
void OnUpdate();
void OnRender();
bool CreateTextureFromDDS
(
    const char * path,
    ComPtr<ID3D11Texture2D> & pTexture,
    ComPtr<ID3D11ShaderResourceView> & pTextureRV
);
DWORD PrintSystemError(DWORD error = GetLastError());

// 画面サイズ
const int Width = 640;
const int Height = 480;

ComPtr<ID3D11Device3> gpDevice;
ComPtr<ID3D11DeviceContext3> gpImmediateContext;
ComPtr<IDXGISwapChain3> gpSwapChain;
ComPtr<ID3D11RenderTargetView> gpRTV;
ComPtr<ID3D11VertexShader> gpVertexShader;
ComPtr<ID3D11PixelShader> gpPixelShader;
ComPtr<ID3D11InputLayout> gpInputLayout;
ComPtr<ID3D11Buffer> gpVertexBuffer;
ComPtr<ID3D11Buffer> gpIndexBuffer;
ComPtr<ID3D11Buffer> gpConstantBuffer;
ComPtr<ID3D11Texture2D> gpTexture;
ComPtr<ID3D11ShaderResourceView> gpTextureRV;
ComPtr<ID3D11SamplerState> gpSampler;

enum Constants
{
    // FourCC 'DDS '
    DDSMagic = 0x20534444,
    DDSHeaderCaps = 0x1,
    DDSHeaderHeight = 0x2,
    DDSHeaderWidth = 0x4,
    DDSHeaderPitch = 0x8,
    DDSHeaderPixelFormat = 0x1000,
    DDSHeaderMipMapCount = 0x20000,
    DDSHeaderLinearSize = 0x80000,
    DDSHeaderDepth = 0x800000,
    DDSHeaderCapsComplex = 0x8,
    DDSHeaderCapsMipMap = 0x400000,
    DDSHeaderCapsTexture = 0x1000,
    DDSHeaderCaps2CubeMap = 0x200,
    DDSHeaderCaps2CubeMapPositiveX = 0x400,
    DDSHeaderCaps2CubeMapNegativeX = 0x800,
    DDSHeaderCaps2CubeMapPositiveY = 0x1000,
    DDSHeaderCaps2CubeMapNegativeY = 0x2000,
    DDSHeaderCaps2CubeMapPositiveZ = 0x4000,
    DDSHeaderCaps2CubeMapNegativeZ = 0x8000,
    DDSHeaderCaps2Volume = 0x200000,
    DDSPixelFormatAlphaPixels = 0x1,
    DDSPixelFormatAlpha = 0x2,
    DDSPixelFormatFourCC = 0x4,
    DDSPixelFormatRGB = 0x40,
    DDSPixelFormatYUV = 0x200,
    DDSPixelFormatLuminance = 0x20000
};

struct DDSPixelFormat
{
    uint32_t Size;
    uint32_t Flags;
    uint32_t FourCC;
    uint32_t RGBBitCount;
    uint32_t RBitMask;
    uint32_t GBitMask;
    uint32_t BBitMask;
    uint32_t ABitMask;
};

struct DDSHeader
{
    uint32_t Size;
    uint32_t Flags;
    uint32_t Height;
    uint32_t Width;
    uint32_t PicthOrLinearSize;
    uint32_t Depth;
    uint32_t MipMapCount;
    uint32_t Reserved1[11];
    DDSPixelFormat PixelFormat;
    uint32_t Caps;
    uint32_t Caps2;
    uint32_t Caps3;
    uint32_t Caps4;
    uint32_t Reserved2;
};

static_assert(sizeof(DDSHeader) == 124, "DDSヘッダのサイズに問題があります．");

struct Vertex
{
    XMFLOAT3 Position;
    XMFLOAT2 Texcoord;
};

struct ConstantBuffer
{
    XMFLOAT4 Color;
};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR lpCmdLine, int nCmdShow)
{
    CD3D11_RECT rect;
    UsingConsole uc;

    // ウィンドウクラスの初期化
    WNDCLASSEX wcx {};
    wcx.cbSize = sizeof wcx;
    wcx.style = CS_HREDRAW | CS_VREDRAW;
    wcx.lpfnWndProc = WindowProc;
    wcx.cbClsExtra = 0;
    wcx.cbWndExtra = 0;
    wcx.hInstance = hInstance;
    wcx.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcx.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcx.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wcx.lpszMenuName = nullptr;
    wcx.lpszClassName = L"LearningDirectX11";

    // ウィンドウクラスの登録
    if(!RegisterClassEx(&wcx))
    {
        PrintSystemError();
        return 0;
    }

    // スクリーンサイズの取得
    const int ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    const int ScreenHeight = GetSystemMetrics(SM_CYSCREEN);

    // ウィンドウサイズの決定 (画面中心に配置)
    DWORD WindowStyle = WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME;
    RECT WindowRect {
        (ScreenWidth - Width) / 2,
        (ScreenHeight - Height) / 2,
        (ScreenWidth + Width) / 2,
        (ScreenHeight + Height) / 2
    };
    AdjustWindowRect(&WindowRect, WindowStyle, FALSE);

    // ウィンドウの生成
    HWND hWnd = CreateWindow(
        wcx.lpszClassName,
        wcx.lpszClassName,
        WindowStyle,
        WindowRect.left,
        WindowRect.top,
        WindowRect.right - WindowRect.left,
        WindowRect.bottom - WindowRect.top,
        nullptr,
        nullptr,
        wcx.hInstance,
        nullptr
    );
    if(!hWnd)
    {
        PrintSystemError();
        return 0;
    }

    // ウィンドウの表示
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // メインループ
    MSG msg {};
    while(msg.message != WM_QUIT)
    {
        // メッセージの確認
        if(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            // メッセージがあった場合の処理
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            // メッセージがなかった場合の処理
            OnUpdate();
            OnRender();
            Sleep(1);
        }
    }

    if(gpImmediateContext)
    {
        gpImmediateContext->ClearState();
    }

    return static_cast<int>(msg.wParam);
}

// メッセージ処理関数
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_CREATE:
        if(!OnCreate(hWnd))
        {
            return -1;
        }
        break;
    case WM_DESTROY:
        // WM_QUITメッセージを送る
        PostQuitMessage(0);
        break;
    default:
        // 未処理のメッセージを処理する
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    return 0;
}

bool OnCreate(HWND hWnd)
{
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

    // ID3D11Device2への変換
    hr = pDevice.As(&gpDevice);
    if(FAILED(hr))
    {
        PrintSystemError(hr);
        return false; 
    }

    // ID3D11DeviceContext2への変換
    hr = pImmediateContext.As(&gpImmediateContext);
    if(FAILED(hr))
    {
        PrintSystemError(hr);
        return false; 
    }

    // DXGIFactory4オブジェクトの取得
    ComPtr<IDXGIFactory4> pFactory;
    {
        ComPtr<IDXGIFactory2> pTmpFactory;
        UINT creationFlag = 0;
#ifdef _DEBUG
        creationFlag |= DXGI_CREATE_FACTORY_DEBUG;
#endif
        hr = CreateDXGIFactory2(creationFlag, IID_PPV_ARGS(pTmpFactory.GetAddressOf()));
        if(FAILED(hr))
        {
            PrintSystemError(hr);
            return false;
        }

        hr = pTmpFactory.As(&pFactory);
        if(FAILED(hr))
        {
            PrintSystemError(hr);
            return false;
        }
    }

    // Alt + Enterによる全画面化の禁止
    pFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);
 
    // SwapChainの生成
    {
        hr = pDevice.As(&gpDevice);
        if(FAILED(hr))
        {
            PrintSystemError(hr);
            return false; 
        }

        DXGI_SWAP_CHAIN_DESC1 SwapChainDesc {};
        SwapChainDesc.Width = Width;
        SwapChainDesc.Height = Height;
        SwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        SwapChainDesc.SampleDesc.Count = 1;
        SwapChainDesc.SampleDesc.Quality = 0;
        SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        SwapChainDesc.BufferCount = 1;

        ComPtr<IDXGISwapChain1> pSwapChain;
        hr = pFactory->CreateSwapChainForHwnd(
            gpDevice.Get(),
            hWnd,
            &SwapChainDesc,
            nullptr,
            nullptr,
            pSwapChain.GetAddressOf()
        );
        if(FAILED(hr))
        {
            PrintSystemError(hr);
            return false;
        }

        hr = pSwapChain.As(&gpSwapChain);
        if(FAILED(hr))
        {
            PrintSystemError(hr);
            return false; 
        }
    }

    // レンダーターゲットビュー (Render Target View : RTV)の生成と設定
    {
        ComPtr<ID3D11Texture2D> pBackBuffer;
        hr = gpSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        if(FAILED(hr))
        {
            PrintSystemError(hr);
            return false;
        }

        hr = gpDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, gpRTV.GetAddressOf());
        if(FAILED(hr))
        {
            PrintSystemError(hr);
            return false;
        }
    }

    // レンダーターゲットの設定
    gpImmediateContext->OMSetRenderTargets(1, gpRTV.GetAddressOf(), nullptr);

    // ビューポートの設定
    D3D11_VIEWPORT viewport {};
    viewport.Width = Width;
    viewport.Height = Height;
    viewport.MinDepth = 0.f;
    viewport.MaxDepth = 1.f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    gpImmediateContext->RSSetViewports(1, &viewport);

    // 頂点シェーダの生成
    {
        ComPtr<ID3DBlob> pVertexShaderBlob;
        ComPtr<ID3DBlob> pErrorBlob;

        hr = D3DCompileFromFile(
            L"010_DDS.hlsl",
            nullptr,
            nullptr,
            "VS",
            "vs_5_0",
            0,
            0,
            pVertexShaderBlob.GetAddressOf(),
            pErrorBlob.GetAddressOf()
        );
        if(FAILED(hr))
        {
            if(pErrorBlob)
            {
                cout << reinterpret_cast<char *>(pErrorBlob->GetBufferPointer()) << endl;
            }
            else
            {
                PrintSystemError(hr);
            }

            return false;
        }

        hr = gpDevice->CreateVertexShader(
            pVertexShaderBlob->GetBufferPointer(),
            pVertexShaderBlob->GetBufferSize(),
            nullptr,
            gpVertexShader.GetAddressOf()
        );
        if(FAILED(hr))
        {
            PrintSystemError(hr);
            return false; 
        }

        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        hr = gpDevice->CreateInputLayout(
            layout,
            _countof(layout),
            pVertexShaderBlob->GetBufferPointer(),
            pVertexShaderBlob->GetBufferSize(),
            gpInputLayout.GetAddressOf()
        );
        if(FAILED(hr))
        {
            cout << static_cast<char *>(pErrorBlob->GetBufferPointer()) << endl;
            return false;
        }

        gpImmediateContext->IASetInputLayout(gpInputLayout.Get());
    }

    // ピクセルシェーダの生成
    {
        ComPtr<ID3DBlob> pPixelShaderBlob;
        ComPtr<ID3DBlob> pErrorBlob;

        hr = D3DCompileFromFile(
            TEXT("010_DDS.hlsl"),
            nullptr,
            nullptr,
            "PS",
            "ps_5_0",
            0,
            0, 
            pPixelShaderBlob.GetAddressOf(),
            pErrorBlob.GetAddressOf()
        );
        if(FAILED(hr))
        {
            if(pErrorBlob)
            {
                cout << reinterpret_cast<char *>(pErrorBlob->GetBufferPointer()) << endl;
            }
            else
            {
                PrintSystemError(hr);
            }
            return false;
        }

        hr = gpDevice->CreatePixelShader(
            pPixelShaderBlob->GetBufferPointer(),
            pPixelShaderBlob->GetBufferSize(),
            nullptr,
            gpPixelShader.GetAddressOf()
        );
        if(FAILED(hr))
        {
            PrintSystemError(hr);
            return false;
        }
    }

    // 頂点バッファの生成
    {
        Vertex vertices[] =
        {
            { XMFLOAT3(-0.5f,  0.5f, 0.5f), XMFLOAT2(0.0f, 0.0f) },
            { XMFLOAT3( 0.5f,  0.5f, 0.5f), XMFLOAT2(1.0f, 0.0f) },
            { XMFLOAT3( 0.5f, -0.5f, 0.5f), XMFLOAT2(1.0f, 1.0f) },
            { XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT2(0.0f, 1.0f) }
        };

        D3D11_BUFFER_DESC BufferDesc {};
        BufferDesc.ByteWidth = sizeof vertices;
        BufferDesc.Usage = D3D11_USAGE_DEFAULT;
        BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        // BufferDesc.CPUAccessFlags = 0;
        // BufferDesc.MiscFlags = 0;
        // BufferDesc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA SubResourceData {};
        SubResourceData.pSysMem = vertices;
        // SubResourceData.SysMemPitch = 0;
        // SubResourceData.SysMemSlicePitch = 0;

        hr = gpDevice->CreateBuffer(&BufferDesc, &SubResourceData, gpVertexBuffer.GetAddressOf());
        if(FAILED(hr))
        {
            PrintSystemError(hr);
            return false; 
        }

        UINT stride = sizeof vertices[0];
        UINT offset = 0;
        gpImmediateContext->IASetVertexBuffers(0, 1, gpVertexBuffer.GetAddressOf(), &stride, &offset);
        gpImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

    // インデックスバッファの生成
    {
        WORD indices[] =
        {
            0, 1, 2,
            0, 2, 3
        };

        D3D11_BUFFER_DESC BufferDesc {};
        BufferDesc.ByteWidth = sizeof indices;
        BufferDesc.Usage = D3D11_USAGE_DEFAULT;
        BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        // BufferDesc.CPUAccessFlags = 0;
        // BufferDesc.MiscFlags = 0;
        // BufferDesc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA SubResourceData {};
        SubResourceData.pSysMem = indices;
        // SubResourceData.SysMemPitch = 0;
        // SubResourceData.SysMemSlicePitch = 0;

        hr = gpDevice->CreateBuffer(&BufferDesc, &SubResourceData, gpIndexBuffer.GetAddressOf());
        if(FAILED(hr))
        {
            PrintSystemError(hr);
            return false;
        }

        gpImmediateContext->IASetIndexBuffer(gpIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
    }

    // 定数バッファの生成
    {
        D3D11_BUFFER_DESC BufferDesc {};
        BufferDesc.ByteWidth = sizeof(ConstantBuffer);
        BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        hr = gpDevice->CreateBuffer(&BufferDesc, nullptr, gpConstantBuffer.GetAddressOf());
        if(FAILED(hr))
        {
            PrintSystemError(hr);
            return false;
        }
    }

    // テクスチャ生成
    {
        if(!CreateTextureFromDDS("sample.dds", gpTexture, gpTextureRV))
        {
            return false;
        }

        D3D11_SAMPLER_DESC SamplerDesc {};
        SamplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
        SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        SamplerDesc.MipLODBias = 0.0f;
        SamplerDesc.MaxAnisotropy = 2;
        SamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        SamplerDesc.BorderColor[0] = 0.0f;
        SamplerDesc.BorderColor[1] = 0.0f;
        SamplerDesc.BorderColor[2] = 0.0f;
        SamplerDesc.BorderColor[3] = 0.0f;
        SamplerDesc.MinLOD = -FLT_MAX;
        SamplerDesc.MaxLOD = FLT_MAX;

        hr = gpDevice->CreateSamplerState(&SamplerDesc, gpSampler.GetAddressOf());
        if(FAILED(hr))
        {
            PrintSystemError(hr);
            return false; 
        }
    }

    return true;
}

void OnUpdate()
{
    HRESULT hr = S_OK;

    static float phi = 0.f;

    D3D11_MAPPED_SUBRESOURCE resource;
    hr = gpImmediateContext->Map(gpConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
    if(FAILED(hr))
    {
        PrintSystemError(hr);
        return;
    }

    {
        ConstantBuffer CB;
        CB.Color = XMFLOAT4((sin(phi) + 1.f) * 0.5f, 0.5f, 0.5f, 1.f);
        phi += 0.005f;
        memcpy(resource.pData, &CB, sizeof CB);
    }
    gpImmediateContext->Unmap(gpConstantBuffer.Get(), 0);

}

void OnRender()
{
    // レンダーターゲットのクリア
    float clearColor[] = { 0.f, 0.f, 0.f, 1.f };
    gpImmediateContext->ClearRenderTargetView(gpRTV.Get(), clearColor);

    gpImmediateContext->VSSetShader(gpVertexShader.Get(), nullptr, 0);
    gpImmediateContext->PSSetShader(gpPixelShader.Get(), nullptr, 0);
    gpImmediateContext->PSSetConstantBuffers(0, 1, gpConstantBuffer.GetAddressOf());
    gpImmediateContext->PSSetShaderResources(0, 1, gpTextureRV.GetAddressOf());
    gpImmediateContext->PSSetSamplers(0, 1, gpSampler.GetAddressOf());
    gpImmediateContext->DrawIndexed(6, 0, 0);

    // 画面へ表示
    DXGI_PRESENT_PARAMETERS parameters {};
    gpSwapChain->Present1(0, 0, &parameters);
}

bool CreateTextureFromDDS
(
    const char * path,
    ComPtr<ID3D11Texture2D> & pTexture,
    ComPtr<ID3D11ShaderResourceView> & pTextureRV
)
{
    ifstream fin(path, ios::in | ios::binary);
    if(!fin)
    {
        return false;
    }

    uint32_t magic;
    fin.read(reinterpret_cast<char *>(&magic), sizeof magic);
    if(magic != DDSMagic)
    {
        return false;
    }

    DDSHeader header;
    fin.read(reinterpret_cast<char *>(&header), sizeof header);
    if(header.Size != sizeof header)
    {
        cout << "想定外のヘッダサイズです．" << endl;
        return false;
    }

    if(!(header.Flags & DDSHeaderCaps))
    {
        cout << "ヘッダにCapsフラグが立っていません．" << endl;
        return false;
    }

    if(!(header.Flags & DDSHeaderHeight))
    {
        cout << "ヘッダにHeightフラグが立っていません．" << endl;
        return false;
    }

    if(!(header.Flags & DDSHeaderWidth))
    {
        cout << "ヘッダにWidthフラグが立っていません．" << endl;
        return false;
    }

    if(!(header.Flags & DDSHeaderPixelFormat))
    {
        cout << "ヘッダにPixelFormatフラグが立っていません．" << endl;
        return false;
    }

    D3D11_TEXTURE2D_DESC Texture2DDesc {};
    Texture2DDesc.Width = header.Width;
    Texture2DDesc.Height = header.Height;
    Texture2DDesc.MipLevels = header.MipMapCount ? header.MipMapCount : 1;
    Texture2DDesc.ArraySize = 1;
    Texture2DDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    Texture2DDesc.SampleDesc.Count = 1;
    Texture2DDesc.SampleDesc.Quality = 0;
    Texture2DDesc.Usage = D3D11_USAGE_IMMUTABLE;
    Texture2DDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    // Texture2DDesc.CPUAccessFlags = 0;
    // Texture2DDesc.MiscFlags = 0;

    vector<uint8_t> buffer(
        (istreambuf_iterator<char>(fin)),
        (istreambuf_iterator<char>())
    );
    D3D11_SUBRESOURCE_DATA TextureData{};
    TextureData.pSysMem = buffer.data();
    TextureData.SysMemPitch = header.Width * 4;

    HRESULT hr = S_OK;

    hr = gpDevice->CreateTexture2D(&Texture2DDesc, &TextureData, pTexture.GetAddressOf());
    if(FAILED(hr))
    {
        PrintSystemError(hr);
        return false;
    }

    hr = gpDevice->CreateShaderResourceView(pTexture.Get(), nullptr, pTextureRV.GetAddressOf());
    if(FAILED(hr))
    {
        PrintSystemError(hr);
        return false;
    }

    return true;
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
