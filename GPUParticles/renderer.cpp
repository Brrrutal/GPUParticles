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

struct RefreshRate
{
    unsigned int m_den = 1;
    unsigned int m_num = 0;
};

RefreshRate GetRefreshRate(const RenderWindow& renderWindow)
{
    RefreshRate refreshRate;

    HRESULT result;
    Microsoft::WRL::ComPtr<IDXGIFactory> pDXGIFactory;
    Microsoft::WRL::ComPtr<IDXGIAdapter> pDXGIAdapter;
    Microsoft::WRL::ComPtr<IDXGIOutput> pDXGIOutput;

    result = CreateDXGIFactory(__uuidof(pDXGIFactory), (void**)pDXGIFactory.GetAddressOf());
    if (FAILED(result))
        return refreshRate;

    result = pDXGIFactory->EnumAdapters(0, pDXGIAdapter.GetAddressOf());
    if (FAILED(result))
        return refreshRate;

    result = pDXGIAdapter->EnumOutputs(0, pDXGIOutput.GetAddressOf());
    if (FAILED(result))
        return refreshRate;

    UINT modeCnt;
    result = pDXGIOutput->GetDisplayModeList(
            DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &modeCnt, NULL);
    if (FAILED(result))
        return refreshRate;

    std::unique_ptr<DXGI_MODE_DESC[]> rDXGIModeArray(new DXGI_MODE_DESC[modeCnt]);
    result = pDXGIOutput->GetDisplayModeList(
            DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &modeCnt, rDXGIModeArray.get());

    if (FAILED(result))
        return refreshRate;

    DXGI_ADAPTER_DESC adapterDesc;
    result = pDXGIAdapter->GetDesc(&adapterDesc);
    if (FAILED(result))
        return refreshRate;

    for (UINT modeIdx = 0; modeIdx < modeCnt; ++modeIdx)
    {
        if (rDXGIModeArray[modeIdx].Width == renderWindow.m_screenWidth &&
            rDXGIModeArray[modeIdx].Height == renderWindow.m_screenHeight)
        {
            refreshRate.m_den = rDXGIModeArray[modeIdx].RefreshRate.Denominator;
            refreshRate.m_num = rDXGIModeArray[modeIdx].RefreshRate.Numerator;
        }
    }

    return refreshRate;
}

Renderer InitializeRenderer(const RenderWindow& renderWindow)
{
    Renderer renderer;

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
    RefreshRate refreshRate = renderWindow.m_verticalSync ?
            GetRefreshRate(renderWindow) : RefreshRate();

    swapChainDesc.BufferDesc.RefreshRate.Numerator = refreshRate.m_num;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = refreshRate.m_den;

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

    // Create the swap chain, Direct3D device, and Direct3D device context.
    result = D3D11CreateDeviceAndSwapChain(
            NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, featureLevels, 1,
            D3D11_SDK_VERSION, &swapChainDesc,
            renderer.m_rSwapChain.GetAddressOf(), 
            renderer.m_rDevice.GetAddressOf(), 
            NULL, renderer.m_rDeviceContext.GetAddressOf());

    if (FAILED(result))
    {
        return renderer;
    }

    // Get back buffer from swap chain.
    Microsoft::WRL::ComPtr<ID3D11Texture2D> rBackBuffer;
    result = renderer.m_rSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)rBackBuffer.GetAddressOf());

    if (FAILED(result))
    {
        return renderer;
    }

    result = renderer.m_rDevice->CreateRenderTargetView(
            rBackBuffer.Get(), NULL, renderer.m_rRenderTargetView.GetAddressOf());

    if (FAILED(result))
    {
        return renderer;
    }
    
    // Building depth target.
    D3D11_TEXTURE2D_DESC depthBufferDesc;
    ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));
    depthBufferDesc.Width = renderWindow.m_screenWidth;
    depthBufferDesc.Height = renderWindow.m_screenHeight;
    depthBufferDesc.MipLevels = 1;
    depthBufferDesc.ArraySize = 1;
    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.SampleDesc.Count = 1;
    depthBufferDesc.SampleDesc.Quality = 0;
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthBufferDesc.CPUAccessFlags = 0;
    depthBufferDesc.MiscFlags = 0;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> rDepthStencilBuffer;
    result = renderer.m_rDevice->CreateTexture2D(&depthBufferDesc, NULL, rDepthStencilBuffer.GetAddressOf());
    if (FAILED(result))
    {
        return renderer;
    }

    // Initialize the depth stencil view.
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

    // Set up the depth stencil view description.
    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    // Create the depth stencil view.
    result = renderer.m_rDevice->CreateDepthStencilView(
            rDepthStencilBuffer.Get(), &depthStencilViewDesc, renderer.m_rDepthStencilView.GetAddressOf());
    if (FAILED(result))
    {
        return renderer;
    }

    // Bind render and depth targets.
    renderer.m_rDeviceContext->OMSetRenderTargets(
            1, renderer.m_rRenderTargetView.GetAddressOf(), renderer.m_rDepthStencilView.Get());

    return renderer;
}

void ClearBuffers(Renderer* pRenderer)
{
    // Clear the back buffer.
    float color[4] = { 1.0f, 0.3f, 0.7f, 1.0f };
    pRenderer->m_rDeviceContext->ClearRenderTargetView(pRenderer->m_rRenderTargetView.Get(), color);

    // Clear the depth buffer.
    pRenderer->m_rDeviceContext->ClearDepthStencilView(pRenderer->m_rDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void Present(Renderer* pRenderer, const RenderWindow& renderWindow)
{
    if (renderWindow.m_verticalSync)
        pRenderer->m_rSwapChain->Present(1, 0);
    else
        pRenderer->m_rSwapChain->Present(0, 0);
}
