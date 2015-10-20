// DX includes.
#include <d3d11.h>
#include <DirectXMath.h>

// COM pointers handling.
#include <wrl.h>

// C++ includes.
#include <memory>

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
        if (rDXGIModeArray[modeIdx].Width == screenWidth &&
            rDXGIModeArray[modeIdx].Height == screenHeight)
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
