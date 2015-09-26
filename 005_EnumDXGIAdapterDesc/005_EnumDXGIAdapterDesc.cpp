// 005_CreateDXGIFactory.cpp
#include <Windows.h>
#include <tchar.h>
#include <wrl/client.h>
#include <d3d11_3.h>
#include <dxgi1_4.h>
#include <iostream>
#include "UsingConsole.h"

#pragma comment(lib, "dxgi.lib")

using namespace Microsoft::WRL;
using namespace std;

ostream & operator<<(ostream & out, const ComPtr<IDXGIAdapter> & pAdapter)
{
    HRESULT hr = S_OK;

    DXGI_ADAPTER_DESC desc {};
    hr = pAdapter->GetDesc(&desc);
    if(FAILED(hr))
    {
        cout << "IDXGIAdapter‚ÌÝ’èŽæ“¾‚ÉŽ¸”s‚µ‚Ü‚µ‚½D" << endl;
        return out;
    }

    cout << "From IDXGIAdapter" << endl;
    wcout << L"Description           : " << desc.Description << endl;
    wcout << L"VendorId              : " << desc.VendorId << endl;
    wcout << L"DeviceId              : " << desc.DeviceId << endl;
    wcout << L"SubSysId              : " << desc.SubSysId << endl;
    wcout << L"Revision              : " << desc.Revision << endl;
    wcout << L"DedicatedVideoMemory  : " << desc.DedicatedVideoMemory << endl;
    wcout << L"DedicatedSystemMemory : " << desc.DedicatedSystemMemory << endl;
    wcout << L"SharedSystemMemory    : " << desc.SharedSystemMemory << endl;
    wcout << L"AdapterLuid           : " << desc.AdapterLuid.HighPart << ", " << desc.AdapterLuid.LowPart << endl;

    return out;
}

ostream & operator<<(ostream & out, const ComPtr<IDXGIAdapter1> & pAdapter)
{
    ComPtr<IDXGIAdapter> adapter;
    if(SUCCEEDED(pAdapter.As(&adapter)))
    {
        cout << adapter << endl;
    }

    DXGI_ADAPTER_DESC1 desc {};
    HRESULT hr = pAdapter->GetDesc1(&desc);
    if(FAILED(hr))
    {
        cout << "IDXGIAdapter1‚ÌÝ’èŽæ“¾‚ÉŽ¸”s‚µ‚Ü‚µ‚½D" << endl;
        return out;
    }

    cout << "From IDXGIAdapter1" << endl;
    wcout << L"Flags : ";

    wchar_t * delim = L"";
    if(desc.Flags == DXGI_ADAPTER_FLAG_NONE)
    {
        wcout << delim << L"None";
        delim = L", ";
    }
    else if(desc.Flags == DXGI_ADAPTER_FLAG_FORCE_DWORD)
    {
        wcout << delim << L"Force DWORD";
        delim = L", ";
    }
    else
    {
        if(desc.Flags & DXGI_ADAPTER_FLAG_REMOTE)
        {
            wcout << delim << L"Remote";
            delim = L", ";
        }

        if(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            wcout << delim << L"Software";
            delim = L", ";
        }
    }

    wcout << endl;

    return out;
}

ostream & operator<<(ostream & out, const ComPtr<IDXGIAdapter2> & pAdapter)
{
    ComPtr<IDXGIAdapter1> adapter;
    if(SUCCEEDED(pAdapter.As(&adapter)))
    {
        cout << adapter << endl;
    }

    DXGI_ADAPTER_DESC2 desc {};
    HRESULT hr = pAdapter->GetDesc2(&desc);
    if(FAILED(hr))
    {
        cout << "IDXGIAdapter2‚ÌÝ’èŽæ“¾‚ÉŽ¸”s‚µ‚Ü‚µ‚½D" << endl;
        return out;
    }

    cout << "From IDXGIAdapter2" << endl;
    static const wchar_t * GraphicsPreemptionGranularities[] =
    {
        L"DMA Buffer Boundary",
        L"Primitive Boundary",
        L"Triangle Boundary",
        L"Pixel Boundary",
        L"Instruction Boundary",
    };

    wcout << L"GraphicsPreemptionGranularity : " <<
        GraphicsPreemptionGranularities[desc.GraphicsPreemptionGranularity] << endl;

    static const wchar_t * ComputePreemptionGranularities[] =
    {
        L"DMA Buffer Boundary",
        L"Dispatch Boundary",
        L"Thread Group Boundary",
        L"Thread Boundary",
        L"Instruction Boundary",
    };
    wcout << L"ComputePreemptionGranularity : " <<
        ComputePreemptionGranularities[desc.ComputePreemptionGranularity] << endl;

    return out;
}

ostream & operator<<(ostream & out, const ComPtr<IDXGIAdapter3> & pAdapter)
{
    ComPtr<IDXGIAdapter2> adapter;
    if(SUCCEEDED(pAdapter.As(&adapter)))
    {
        cout << adapter << endl;
    }

    return out;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR lpCmdLine, int nCmdShow)
{
    UsingConsole uc;

    HRESULT hr = S_OK;

    // CreateDXGIFactory2‚É‚æ‚éDXGIFactory2‚Ì¶¬
    ComPtr<IDXGIFactory4> pFactory4;
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

        hr = pFactory2.As(&pFactory4);
        if(FAILED(hr)) { return 0; }
    }

    UINT index = 0;
    while(true)
    {
        ComPtr<IDXGIAdapter1> pAdapter1;
        hr = pFactory4->EnumAdapters1(index, pAdapter1.GetAddressOf());

        if(hr == DXGI_ERROR_NOT_FOUND)
        {
            break;
        }
        else if(FAILED(hr))
        {
            return 0;
        }

        cout << "Adapter[" << index << "]" << endl;
        ++index;

        ComPtr<IDXGIAdapter2> pAdapter2;
        ComPtr<IDXGIAdapter3> pAdapter3;
        if(SUCCEEDED(pAdapter1.As(&pAdapter3)))
        {
            cout << pAdapter3 << endl;
        }
        else if(SUCCEEDED(pAdapter1.As(&pAdapter2)))
        {
            cout << pAdapter2 << endl;
        }
        else
        {
            cout << pAdapter1 << endl;
        }
    }

    return 0;
}