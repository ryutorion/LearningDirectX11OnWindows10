// 014_DiffuseReflection.cpp
#include <Windows.h>
#include <tchar.h>
#include <iostream>
#include <cstdlib>
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
ComPtr<ID3D11Buffer> gpLightBuffer;
ComPtr<ID3D11Texture2D> gpTexture;
ComPtr<ID3D11ShaderResourceView> gpTextureRV;
ComPtr<ID3D11SamplerState> gpSampler;
ComPtr<ID3D11RasterizerState2> gpRasterizerState;
ComPtr<ID3D11BlendState1> gpBlendState;

struct Vertex
{
    XMFLOAT3 Position;
    XMFLOAT3 Normal;
};

struct ConstantBuffer
{
    XMMATRIX Model;
    XMMATRIX View;
    XMMATRIX Projection;
};

struct LightBuffer
{
    XMFLOAT3 LightDir;
    XMFLOAT3 LightColor;
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
        ComPtr<IDXGIDevice> pDXGIDevice;
        hr = pDevice.As(&pDXGIDevice);
        if(FAILED(hr))
        {
            PrintSystemError(hr);
            return false;
        }

        ComPtr<IDXGIAdapter> pDXGIAdapter;
        hr = pDXGIDevice->GetAdapter(pDXGIAdapter.GetAddressOf());
        if(FAILED(hr))
        {
            PrintSystemError(hr);
            return false;
        }

        hr = pDXGIAdapter->GetParent(IID_PPV_ARGS(pFactory.GetAddressOf()));
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
            L"014.hlsl",
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
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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
            TEXT("014.hlsl"),
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

    WORD indices[] =
    {
        // 前面
        0, 1, 2,
        0, 2, 3,

        // 右側面
        1, 5, 2,
        5, 6, 2,

        // 背面
        5, 7, 6,
        5, 4, 7,

        // 左側面
        0, 3, 7,
        0, 7, 4,

        // 上面
        0, 5, 1,
        0, 4, 5,

        // 底面
        3, 2, 7,
        2, 6, 7,
    };

    // 頂点バッファの生成
    {
        Vertex vertices[] =
        {
            { XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
            { XMFLOAT3( 0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
            { XMFLOAT3( 0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
            { XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
            { XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
            { XMFLOAT3( 0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
            { XMFLOAT3( 0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
            { XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
        };

        // 法線の計算
        for(auto i = 0; i < _countof(indices); i += 3)
        {
            auto & v0 = vertices[indices[i + 0]];
            auto & v1 = vertices[indices[i + 1]];
            auto & v2 = vertices[indices[i + 2]];

            auto a = XMVectorSet(v0.Position.x, v0.Position.y, v0.Position.z, 0.0f);
            auto b = XMVectorSet(v1.Position.x, v1.Position.y, v1.Position.z, 0.0f);
            auto c = XMVectorSet(v2.Position.x, v2.Position.y, v2.Position.z, 0.0f);
            auto n = XMVector3Normalize(XMVector3Cross(c - a, b - a));

            v0.Normal.x += XMVectorGetX(n);
            v0.Normal.y += XMVectorGetY(n);
            v0.Normal.z += XMVectorGetZ(n);
            v1.Normal.x += XMVectorGetX(n);
            v1.Normal.y += XMVectorGetY(n);
            v1.Normal.z += XMVectorGetZ(n);
            v2.Normal.x += XMVectorGetX(n);
            v2.Normal.y += XMVectorGetY(n);
            v2.Normal.z += XMVectorGetZ(n);
        }

        // 法線の正規化
        for(auto i = 0; i < _countof(vertices); ++i)
        {
            auto & v = vertices[i];
            auto n = XMVector3Normalize(XMVectorSet(v.Normal.x, v.Normal.y, v.Normal.z, 0.0f));

            v.Normal.x = XMVectorGetX(n);
            v.Normal.y = XMVectorGetY(n);
            v.Normal.z = XMVectorGetZ(n);
        }

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
        BufferDesc.ByteWidth = (sizeof(ConstantBuffer) + 15) & ~0xf;
        BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        hr = gpDevice->CreateBuffer(&BufferDesc, nullptr, gpConstantBuffer.GetAddressOf());
        if(FAILED(hr))
        {
            PrintSystemError(hr);
            return false;
        }

        BufferDesc.ByteWidth = (sizeof(LightBuffer) + 15) & ~0xf;
        hr = gpDevice->CreateBuffer(&BufferDesc, nullptr, gpLightBuffer.GetAddressOf());
        if(FAILED(hr))
        {
            PrintSystemError(hr);
            return false;
        }
    }

    // テクスチャ生成
    {
        D3D11_TEXTURE2D_DESC Texture2DDesc {};
        Texture2DDesc.Width = 128;
        Texture2DDesc.Height = 128;
        Texture2DDesc.MipLevels = 1;
        Texture2DDesc.ArraySize = 1;
        Texture2DDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        Texture2DDesc.SampleDesc.Count = 1;
        Texture2DDesc.SampleDesc.Quality = 0;
        Texture2DDesc.Usage = D3D11_USAGE_IMMUTABLE;
        Texture2DDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        // Texture2DDesc.CPUAccessFlags = 0;
        // Texture2DDesc.MiscFlags = 0;

        uint32_t texture[128][128];
        uint32_t color1 = ~0u ^ (0xff << 8);
        uint32_t color2 = 0 | (0xff << 16);
        for(auto h = 0; h < 128; ++h)
        {
            auto r = h / 16;
            for(auto w = 0; w < 128; ++w)
            {
                auto c = w / 16;
                if((c & 1 + r & 1) & 1)
                {
                    texture[h][w] = color1;
                }
                else
                {
                    texture[h][w] = color2;
                }
            }
        }


        D3D11_SUBRESOURCE_DATA TextureData {};
        TextureData.pSysMem = texture;
        TextureData.SysMemPitch = 128 * 4;

        hr = gpDevice->CreateTexture2D(&Texture2DDesc, &TextureData, gpTexture.GetAddressOf());
        if(FAILED(hr))
        {
            PrintSystemError(hr);
            return false;
        }

        hr = gpDevice->CreateShaderResourceView(gpTexture.Get(), nullptr, gpTextureRV.GetAddressOf());
        if(FAILED(hr))
        {
            PrintSystemError(hr);
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

    // RasterizerStateの生成
    {
        D3D11_RASTERIZER_DESC2 RasterizerDesc {};
        // デフォルト設定
        // RasterizerDesc.FillMode = D3D11_FILL_SOLID;
        // RasterizerDesc.CullMode = D3D11_CULL_BACK;
        // RasterizerDesc.FrontCounterClockwise = FALSE;
        // RasterizerDesc.DepthBias = 0;
        // RasterizerDesc.DepthBiasClamp = 0.0f;
        // RasterizerDesc.SlopeScaledDepthBias = 0.0f;
        // RasterizerDesc.DepthClipEnable = TRUE;
        // RasterizerDesc.ScissorEnable = FALSE;
        // RasterizerDesc.MultisampleEnable = FALSE;
        // RasterizerDesc.AntialiasedLineEnable = FALSE;
        // RasterizerDesc.ForcedSampleCount = 0;
        // RasterizerDesc.ConservativeRaster = D3D11_CONSERVATIVE_RASTERIZATION_MODE_OFF;
        RasterizerDesc.FillMode = D3D11_FILL_SOLID;
        RasterizerDesc.CullMode = D3D11_CULL_BACK;
        RasterizerDesc.FrontCounterClockwise = FALSE;
        RasterizerDesc.DepthBias = 0;
        RasterizerDesc.DepthBiasClamp = 0.0f;
        RasterizerDesc.SlopeScaledDepthBias = 0.0f;
        RasterizerDesc.DepthClipEnable = TRUE;
        RasterizerDesc.ScissorEnable = FALSE;
        RasterizerDesc.MultisampleEnable = FALSE;
        RasterizerDesc.AntialiasedLineEnable = FALSE;
        RasterizerDesc.ForcedSampleCount = 0;
        RasterizerDesc.ConservativeRaster = D3D11_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        hr = gpDevice->CreateRasterizerState2(&RasterizerDesc, gpRasterizerState.GetAddressOf());
        if(FAILED(hr))
        {
            PrintSystemError(hr);
            return false;
        }
    }

    // BlendStateの生成
    {
        D3D11_BLEND_DESC1 BlendDesc {};
        // デフォルト設定
        // BlendDesc.AlphaToCoverageEnable = FALSE;
        // BlendDesc.IndependentBlendEnable = FALSE;
        // for(auto & RTBlendDesc : BlendDesc.RenderTarget)
        // {
        //     RTBlendDesc.BlendEnable = FALSE;
        //     RTBlendDesc.LogicOpEnable = FALSE;
        //     RTBlendDesc.SrcBlend = D3D11_BLEND_ONE;
        //     RTBlendDesc.DestBlend = D3D11_BLEND_ZERO;
        //     RTBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
        //     RTBlendDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
        //     RTBlendDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
        //     RTBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
        //     RTBlendDesc.LogicOp = D3D11_LOGIC_OP_NOOP;
        //     RTBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        // }
        BlendDesc.AlphaToCoverageEnable = FALSE;
        BlendDesc.IndependentBlendEnable = FALSE;

        // ブレンドファクターの有効無効および値の指定
        auto & RTBlendDesc = BlendDesc.RenderTarget[0];
        RTBlendDesc.BlendEnable = TRUE;
        RTBlendDesc.LogicOpEnable = FALSE;
        RTBlendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
        RTBlendDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        RTBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
        RTBlendDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
        RTBlendDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
        RTBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
        RTBlendDesc.LogicOp = D3D11_LOGIC_OP_NOOP;
        RTBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        hr = gpDevice->CreateBlendState1(&BlendDesc, gpBlendState.GetAddressOf());
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
    static float theta = 0.f;
    static XMVECTOR Eye = XMVectorSet(0.0f, 0.0f, 3.0f, 1.0f);
    static XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    static XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);

    static float nearPlane = 0.001f;
    static float farPlane = 1000.0f;
    static float aspect = static_cast<float>(Width) / Height;
    static float fov = 45.0f;
    static auto worldLightDir = XMVector3Normalize(XMVectorSet(1.0f, 1.0f, 0.0f, 0.0f));

    ConstantBuffer CB {};
    CB.Model = DirectX::XMMatrixRotationY(phi) * DirectX::XMMatrixRotationX(theta);
    phi += 0.0015f;
    theta += 0.0015f;
    CB.View = DirectX::XMMatrixLookAtRH(Eye, At, Up);
    CB.Projection = DirectX::XMMatrixPerspectiveFovRH(fov, aspect, nearPlane, farPlane);

    D3D11_MAPPED_SUBRESOURCE resource;
    hr = gpImmediateContext->Map(gpConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
    if(FAILED(hr))
    {
        PrintSystemError(hr);
        return;
    }

    memcpy(resource.pData, &CB, sizeof CB);
    gpImmediateContext->Unmap(gpConstantBuffer.Get(), 0);

    LightBuffer LB {};
    auto localLightDir = XMVector3TransformCoord(worldLightDir, XMMatrixInverse(nullptr, CB.Model));

    LB.LightDir.x = XMVectorGetX(localLightDir);
    LB.LightDir.y = XMVectorGetY(localLightDir);
    LB.LightDir.z = XMVectorGetZ(localLightDir);
    LB.LightColor = XMFLOAT3(0.5f, 0.5f, 1.0f);

    hr = gpImmediateContext->Map(gpLightBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
    if(FAILED(hr))
    {
        PrintSystemError(hr);
        return;
    }

    memcpy(resource.pData, &LB, sizeof LB);
    gpImmediateContext->Unmap(gpLightBuffer.Get(), 0);
}

void OnRender()
{
    // レンダーターゲットのクリア
    float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    gpImmediateContext->ClearRenderTargetView(gpRTV.Get(), clearColor);

    // Vertex Shader Stageの設定
    gpImmediateContext->VSSetShader(gpVertexShader.Get(), nullptr, 0);
    gpImmediateContext->VSSetConstantBuffers(0, 1, gpConstantBuffer.GetAddressOf());

    // Rasterizer Stageの設定
    gpImmediateContext->RSSetState(gpRasterizerState.Get());

    // Pixel Shader Stageの設定
    gpImmediateContext->PSSetShader(gpPixelShader.Get(), nullptr, 0);
    gpImmediateContext->PSSetShaderResources(0, 1, gpTextureRV.GetAddressOf());
    gpImmediateContext->PSSetSamplers(0, 1, gpSampler.GetAddressOf());
    gpImmediateContext->PSSetConstantBuffers(1, 1, gpLightBuffer.GetAddressOf());

    // Output Merger Stageの設定
    gpImmediateContext->OMSetBlendState(gpBlendState.Get(), nullptr, ~0u);


    gpImmediateContext->DrawIndexed(36, 0, 0);

    // 画面へ表示
    DXGI_PRESENT_PARAMETERS parameters {};
    gpSwapChain->Present1(0, 0, &parameters);
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
