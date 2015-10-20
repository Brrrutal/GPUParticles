#pragma once

#include <windows.h>
#include <wrl.h>
#include <d3d11.h>

struct RenderWindow
{
    bool m_fullScreenMode;
    bool m_verticalSync;
    unsigned int m_screenWidth;
    unsigned int m_screenHeight;
    HINSTANCE m_hInstance;
    HWND m_windowHandle;
};

struct AdapterInfo
{
    unsigned int m_den;
    unsigned int m_num;
    unsigned long long m_videoMemoryMB;
};

struct Renderer
{
    Microsoft::WRL::ComPtr<ID3D11Device> m_rDevice;
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_rSwapChain;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_rDeviceContext;
};
