// 005_CreateDXGIFactory.cpp
#include <Windows.h>
#include <tchar.h>
#include <wrl/client.h>
#include <d3d11_3.h>
#include <dxgi1_4.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

using namespace Microsoft::WRL;
using namespace std;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR lpCmdLine, int nCmdShow)
{
    HRESULT hr = S_OK;

    // CreateDXGIFactoryによるDXGIFactoryの生成
    {
        ComPtr<IDXGIFactory> pFactory;
        hr = CreateDXGIFactory(IID_PPV_ARGS(pFactory.GetAddressOf()));
        if(FAILED(hr)) { return 0; }

        // インターフェースがサポートされていない，と失敗する
        // ComPtr<IDXGIFactory1> pFactory1;
        // hr = pFactory.As(&pFactory1);
        // if(FAILED(hr)) { return 0; }
    }

    // CreateDXGIFactory1によるDXGIFactory1の生成
    {
        ComPtr<IDXGIFactory1> pFactory1;
        hr = CreateDXGIFactory1(IID_PPV_ARGS(pFactory1.GetAddressOf()));
        if(FAILED(hr)) { return 0; }

        ComPtr<IDXGIFactory> pFactory;
        hr = pFactory1.As(&pFactory);
        if(FAILED(hr)) { return 0; }

        ComPtr<IDXGIFactory3> pFactory3;
        hr = pFactory1.As(&pFactory3);
        if(FAILED(hr)) { return 0; }

        ComPtr<IDXGIFactory4> pFactory4;
        hr = pFactory1.As(&pFactory4);
        if(FAILED(hr)) { return 0; }
    }

    // CreateDXGIFactory2によるDXGIFactory2の生成
    {
        ComPtr<IDXGIFactory2> pFactory2;
        ULONG creationFlag = 0;
#ifdef _DEBUG
        creationFlag |= DXGI_CREATE_FACTORY_DEBUG;
#endif

        hr = CreateDXGIFactory2(creationFlag, IID_PPV_ARGS(pFactory2.GetAddressOf()));
        if(FAILED(hr)) { return 0; }

        ComPtr<IDXGIFactory> pFactory;
        hr = pFactory2.As(&pFactory);
        if(FAILED(hr)) { return 0; }

        ComPtr<IDXGIFactory1> pFactory1;
        hr = pFactory2.As(&pFactory1);
        if(FAILED(hr)) { return 0; }

        ComPtr<IDXGIFactory3> pFactory3;
        hr = pFactory2.As(&pFactory3);
        if(FAILED(hr)) { return 0; }

        ComPtr<IDXGIFactory4> pFactory4;
        hr = pFactory2.As(&pFactory4);
        if(FAILED(hr)) { return 0; }
    }

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
    if(FAILED(hr)) { return 0; }

    ComPtr<IDXGIDevice> pDXGIDevice;
    hr = pDevice.As(&pDXGIDevice);
    if(FAILED(hr)) { return 0; }

    // D3D11DeviceからDXGIDeviceを取得し，
    // DXGIDeviceからDXGIAdapterを取得し，
    // DXGIAdapterの親であるDXGIFactoryを取得する．
    // そこからIDXGIFactory1，2，3，4を取得する．
    {
        ComPtr<IDXGIAdapter> pDXGIAdapter;
        hr = pDXGIDevice->GetAdapter(pDXGIAdapter.GetAddressOf());
        if(FAILED(hr)) { return 0; }

        ComPtr<IDXGIFactory> pFactory;
        hr = pDXGIAdapter->GetParent(IID_PPV_ARGS(pFactory.GetAddressOf()));
        if(FAILED(hr)) { return 0; }

        ComPtr<IDXGIFactory1> pFactory1;
        hr = pFactory.As(&pFactory1);
        if(FAILED(hr)) { return 0; }

        ComPtr<IDXGIFactory2> pFactory2;
        hr = pFactory.As(&pFactory2);
        if(FAILED(hr)) { return 0; }

        ComPtr<IDXGIFactory3> pFactory3;
        hr = pFactory.As(&pFactory3);
        if(FAILED(hr)) { return 0; }

        ComPtr<IDXGIFactory4> pFactory4;
        hr = pFactory.As(&pFactory4);
        if(FAILED(hr)) { return 0; }
    }

    return 0;
}