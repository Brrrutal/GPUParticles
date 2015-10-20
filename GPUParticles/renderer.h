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

struct RefreshRate
{
    unsigned int m_den;
    unsigned int m_num;
};

struct Renderer
{
    Microsoft::WRL::ComPtr<ID3D11Device> m_rDevice;
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_rSwapChain;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_rDeviceContext;

    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rRenderTargetView;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_rDepthStencilView;
};

Renderer InitializeRenderer(const RenderWindow& renderWindow);
void ClearBuffers(Renderer* pRenderer);
void Present(Renderer* pRenderer, const RenderWindow& renderWindow);
