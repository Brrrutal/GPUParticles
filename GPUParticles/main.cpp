#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "renderer.h"

// System-related storage.
struct App
{
    RenderWindow m_renderWindow;
    LPCWSTR m_name;
};

// Windows message processing.
LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
    switch (umessage)
    {
    // Check if the window is being destroyed.
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    }

    // Check if the window is being closed.
    case WM_CLOSE:
    {
        PostQuitMessage(0);
        return 0;
    } 
    // Check if a key has been pressed on the keyboard.
    case WM_KEYDOWN:
    {
        // TODO: Process keyboard events.
        return 0;
    }

    // Check if a key has been released on the keyboard.
    case WM_KEYUP:
    {
        // TODO: Process keyboard events.
        return 0;
    }

    // Any other messages send to the default message handler as our application won't make use of them.
    default:
    {
        return DefWindowProc(hwnd, umessage, wparam, lparam);
    }
    }
}

// Windows application initialization.
App InitializeApp(bool fullScreen)
{
    App app;
    app.m_name = L"GPU Particles";
    app.m_renderWindow.m_fullScreenMode = fullScreen;

    WNDCLASSEX wc;
    DEVMODE dmScreenSettings;
    int posX, posY;

    app.m_renderWindow.m_hInstance = GetModuleHandle(NULL);

    // Setup the windows class with default settings.
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = app.m_renderWindow.m_hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    wc.hIconSm = wc.hIcon;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = app.m_name;
    wc.cbSize = sizeof(WNDCLASSEX);

    // Register the window class.
    RegisterClassEx(&wc);

    // Determine the resolution of the clients desktop screen.
    app.m_renderWindow.m_screenWidth = GetSystemMetrics(SM_CXSCREEN);
    app.m_renderWindow.m_screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Setup the screen settings depending on whether it is running in full screen or in windowed mode.
    if (app.m_renderWindow.m_fullScreenMode)
    {
        // If full screen set the screen to maximum size of the users desktop and 32bit.
        memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
        dmScreenSettings.dmSize = sizeof(dmScreenSettings);
        dmScreenSettings.dmPelsWidth = (unsigned long)app.m_renderWindow.m_screenWidth;
        dmScreenSettings.dmPelsHeight = (unsigned long)app.m_renderWindow.m_screenHeight;
        dmScreenSettings.dmBitsPerPel = 32;
        dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

        // Change the display settings to full screen.
        ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

        // Set the position of the window to the top left corner.
        posX = posY = 0;
    }
    else
    {
        // If windowed then set it to 800x600 resolution.
        app.m_renderWindow.m_screenWidth = 800;
        app.m_renderWindow.m_screenHeight = 600;

        // Place the window in the middle of the screen.
        posX = (GetSystemMetrics(SM_CXSCREEN) - app.m_renderWindow.m_screenWidth) / 2;
        posY = (GetSystemMetrics(SM_CYSCREEN) - app.m_renderWindow.m_screenHeight) / 2;
    }

    // Create the window with the screen settings and get the handle to it.
    app.m_renderWindow.m_windowHandle = CreateWindowEx(WS_EX_APPWINDOW, app.m_name, app.m_name,
            WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
            posX, posY, 
            app.m_renderWindow.m_screenWidth, app.m_renderWindow.m_screenHeight, 
            NULL, NULL, app.m_renderWindow.m_hInstance, NULL);

    // Bring the window up on the screen and set it as main focus.
    ShowWindow(app.m_renderWindow.m_windowHandle, SW_SHOW);
    SetForegroundWindow(app.m_renderWindow.m_windowHandle);
    SetFocus(app.m_renderWindow.m_windowHandle);

    // Hide the mouse cursor.
    ShowCursor(false);

    return app;
}

// Shutdown application.
void ShutdownApp(App* pApp)
{
    // Show the mouse cursor.
    ShowCursor(true);

    // Fix the display settings if leaving full screen mode.
    if (pApp->m_renderWindow.m_fullScreenMode)
    {
        ChangeDisplaySettings(NULL, 0);
    }

    // Remove the window.
    DestroyWindow(pApp->m_renderWindow.m_windowHandle);
    pApp->m_renderWindow.m_windowHandle = NULL;

    // Remove the application instance.
    UnregisterClass(pApp->m_name, pApp->m_renderWindow.m_hInstance);
    pApp->m_renderWindow.m_hInstance = NULL;
}

// Main application loop.
void RunLoop(Renderer* pRenderer, const RenderWindow& renderWindow)
{
    MSG msg;

    // Initialize the message structure.
    ZeroMemory(&msg, sizeof(MSG));

    // Loop until there is a quit message from the window or the user.
    while (1)
    {
        // Handle the windows messages.
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // If windows signals to end the application then exit out.
        if (msg.message == WM_QUIT)
        {
            return;
        }
        else
        {
            // Otherwise do the frame processing.
            ClearBuffers(pRenderer);
            Present(pRenderer, renderWindow);
        }

    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int cmdShow)
{
    App app = InitializeApp(false);
    Renderer renderer = InitializeRenderer(app.m_renderWindow);
    RunLoop(&renderer, app.m_renderWindow);
    ShutdownApp(&app);
	return 0;
}
