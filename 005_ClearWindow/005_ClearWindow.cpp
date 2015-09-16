// 005_ClearWindow.cpp
#include <Windows.h>
#include <tchar.h>
#include <iostream>
#include <cstdlib>
#include <d3d11_3.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
#include "UsingConsole.h"

using namespace std;
using namespace Microsoft::WRL;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool OnCreate(HWND hWnd);
void OnRender();
DWORD PrintSystemError(DWORD error = GetLastError());

// 画面サイズ
const int Width = 640;
const int Height = 480;

ComPtr<ID3D11Device3> gpDevice;
ComPtr<ID3D11DeviceContext3> gpImmediateContext;
ComPtr<IDXGISwapChain3> gpSwapChain;
ComPtr<ID3D11RenderTargetView> gpRTV;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR lpCmdLine, int nCmdShow)
{
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

    return true;
}

void OnRender()
{
    // レンダーターゲットのクリア
    float clearColor[] = { 0.f, 0.f, 0.f, 1.f };
    gpImmediateContext->ClearRenderTargetView(gpRTV.Get(), clearColor);

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
