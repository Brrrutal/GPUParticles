// DX includes.
#include <DirectXMath.h>

// COM pointers handling.
#include <wrl.h>

// C++ includes.
#include <memory>

#include "renderer.h"

// Linking DX Libraries.
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

bool GetAdapterInfo(const RenderWindow& renderWindow, AdapterInfo* pAdapterInfo)
{
    if (pAdapterInfo == NULL)
        return false;

    HRESULT result;
    Microsoft::WRL::ComPtr<IDXGIFactory> pDXGIFactory;
    Microsoft::WRL::ComPtr<IDXGIAdapter> pDXGIAdapter;
    Microsoft::WRL::ComPtr<IDXGIOutput> pDXGIOutput;

    result = CreateDXGIFactory(__uuidof(pDXGIFactory), (void**)pDXGIFactory.GetAddressOf());
    if (FAILED(result))
        return false;

    result = pDXGIFactory->EnumAdapters(0, pDXGIAdapter.GetAddressOf());
    if (FAILED(result))
        return false;

    result = pDXGIAdapter->EnumOutputs(0, pDXGIOutput.GetAddressOf());
    if (FAILED(result))
        return false;

    UINT modeCnt;
    result = pDXGIOutput->GetDisplayModeList(
            DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &modeCnt, NULL);
    if (FAILED(result))
        return false;

    std::unique_ptr<DXGI_MODE_DESC[]> rDXGIModeArray(new DXGI_MODE_DESC[modeCnt]);
    result = pDXGIOutput->GetDisplayModeList(
            DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &modeCnt, rDXGIModeArray.get());

    if (FAILED(result))
        return false;

    for (UINT modeIdx = 0; modeIdx < modeCnt; ++modeIdx)
    {
        if (rDXGIModeArray[modeIdx].Width == renderWindow.m_screenWidth &&
            rDXGIModeArray[modeIdx].Height == renderWindow.m_screenHeight)
        {
            pAdapterInfo->m_den = rDXGIModeArray[modeIdx].RefreshRate.Denominator;
            pAdapterInfo->m_num = rDXGIModeArray[modeIdx].RefreshRate.Numerator;
        }
    }
    
    DXGI_ADAPTER_DESC adapterDesc;
    result = pDXGIAdapter->GetDesc(&adapterDesc);
    if (FAILED(result))
        return false;

    pAdapterInfo->m_videoMemoryMB = adapterDesc.DedicatedVideoMemory / 1024 / 1024;

    return true;
}

void InitializeRenderer(const RenderWindow& renderWindow)
{
    AdapterInfo adapterInfo;
    if (!GetAdapterInfo(renderWindow, &adapterInfo))
        return;

    HRESULT result;
    DXGI_SWAP_CHAIN_DESC swapChainDesc;

    // Initialize the swap chain description.
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

    // Set to a single back buffer.
    swapChainDesc.BufferCount = 1;

    // Set the width and height of the back buffer.
    swapChainDesc.BufferDesc.Width = renderWindow.m_screenWidth;
    swapChainDesc.BufferDesc.Height = renderWindow.m_screenHeight;

    // Set regular 32-bit surface for the back buffer.
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    // Set the refresh rate of the back buffer.
    if (renderWindow.m_verticalSync)
    {
        swapChainDesc.BufferDesc.RefreshRate.Numerator = adapterInfo.m_num;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = adapterInfo.m_den;
    }
    else
    {
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    }

    // Set the usage of the back buffer.
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    // Set the handle for the window to render to.
    swapChainDesc.OutputWindow = renderWindow.m_windowHandle;

    // Turn multisampling off.
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;

    swapChainDesc.Windowed = !renderWindow.m_fullScreenMode;

    // Set the scan line ordering and scaling to unspecified.
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    // Discard the back buffer contents after presenting.
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    // Don't set the advanced flags.
    swapChainDesc.Flags = 0;

    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };

    Renderer renderer;

    // Create the swap chain, Direct3D device, and Direct3D device context.
    result = D3D11CreateDeviceAndSwapChain(
            NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, featureLevels, 1,
            D3D11_SDK_VERSION, &swapChainDesc,
            renderer.m_rSwapChain.GetAddressOf(), 
            renderer.m_rDevice.GetAddressOf(), 
            NULL, renderer.m_rDeviceContext.GetAddressOf());

    if (FAILED(result))
    {
        return;
    }
}
