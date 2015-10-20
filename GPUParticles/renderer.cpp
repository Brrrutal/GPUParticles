#include <d3d11.h>
#include <DirectXMath.h>

// Linking DX Libraries.
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

struct AdapterInfo
{
    unsigned int m_den;
    unsigned int m_num;
    unsigned long long m_videoMemoryMB;
};

bool GetAdapterInfo(unsigned int screenWidth, unsigned int screenHeight, AdapterInfo* pAdapterInfo)
{
    if (pAdapterInfo == NULL)
        return false;

    HRESULT result;
    IDXGIFactory* pDXGIFactory;
    IDXGIAdapter* pDXGIAdapter;
    IDXGIOutput* pDXGIOutput;

    UINT modeCnt;
    DXGI_MODE_DESC* pDXGIModeArray;

    // TODO: Fix memory leaks in COM-objects.

    result = CreateDXGIFactory(__uuidof(pDXGIFactory), (void**)&pDXGIFactory);
    if (FAILED(result))
        return false;

    result = pDXGIFactory->EnumAdapters(0, &pDXGIAdapter);
    if (FAILED(result))
        return false;

    result = pDXGIAdapter->EnumOutputs(0, &pDXGIOutput);
    if (FAILED(result))
        return false;

    result = pDXGIOutput->GetDisplayModeList(
            DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &modeCnt, NULL);
    if (FAILED(result))
        return false;

    pDXGIModeArray = new DXGI_MODE_DESC[modeCnt];
    result = pDXGIOutput->GetDisplayModeList(
            DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &modeCnt, pDXGIModeArray);

    if (FAILED(result))
        return false;

    for (UINT modeIdx = 0; modeIdx < modeCnt; ++modeIdx)
    {
        if (pDXGIModeArray[modeIdx].Width == screenWidth &&
            pDXGIModeArray[modeIdx].Height == screenHeight)
        {
            pAdapterInfo->m_den = pDXGIModeArray[modeIdx].RefreshRate.Denominator;
            pAdapterInfo->m_num = pDXGIModeArray[modeIdx].RefreshRate.Numerator;
        }
    }
    
    DXGI_ADAPTER_DESC adapterDesc;
    result = pDXGIAdapter->GetDesc(&adapterDesc);
    if (FAILED(result))
        return false;

    pAdapterInfo->m_videoMemoryMB = adapterDesc.DedicatedVideoMemory / 1024 / 1024;
    
    delete[] pDXGIModeArray;
    pDXGIOutput->Release();
    pDXGIAdapter->Release();
    pDXGIFactory->Release();

    return true;
}
