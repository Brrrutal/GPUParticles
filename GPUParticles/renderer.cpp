#include <d3d11.h>
#include <DirectXMath.h>

// Linking DX Libraries.
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

void InitializeRenderer(unsigned int screenWidth, unsigned int screenHeight)
{
    HRESULT result;
    IDXGIFactory* pDXGIFactory;
    IDXGIAdapter* pDXGIAdapter;
    IDXGIOutput* pDXGIOutput;

    UINT modeCnt;
    DXGI_MODE_DESC* pDXGIModeArray;

    unsigned int den = 0;
    unsigned int num = 1;

    // TODO: Fix memory leaks in COM-objects.

    result = CreateDXGIFactory(__uuidof(pDXGIFactory), (void**)&pDXGIFactory);
    if (FAILED(result))
        return;

    result = pDXGIFactory->EnumAdapters(0, &pDXGIAdapter);
    if (FAILED(result))
        return;

    result = pDXGIAdapter->EnumOutputs(0, &pDXGIOutput);
    if (FAILED(result))
        return;

    result = pDXGIOutput->GetDisplayModeList(
            DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &modeCnt, NULL);
    if (FAILED(result))
        return;

    pDXGIModeArray = new DXGI_MODE_DESC[modeCnt];
    result = pDXGIOutput->GetDisplayModeList(
            DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &modeCnt, pDXGIModeArray);

    if (FAILED(result))
        return;

    for (UINT modeIdx = 0; modeIdx < modeCnt; ++modeIdx)
    {
        if (pDXGIModeArray[modeIdx].Width == screenWidth &&
            pDXGIModeArray[modeIdx].Height == screenHeight)
        {
            den = pDXGIModeArray[modeIdx].RefreshRate.Denominator;
            num = pDXGIModeArray[modeIdx].RefreshRate.Numerator;
        }
    }
    
    DXGI_ADAPTER_DESC adapterDesc;
    result = pDXGIAdapter->GetDesc(&adapterDesc);
    if (FAILED(result))
        return;

    unsigned long long videoMemoryMB = adapterDesc.DedicatedVideoMemory / 1024 / 1024;
    
    delete[] pDXGIModeArray;
    pDXGIOutput->Release();
    pDXGIAdapter->Release();
    pDXGIFactory->Release();
}
