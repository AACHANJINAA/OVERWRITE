#include <windows.h>
#include <gl/GL.h>

namespace
{
constexpr wchar_t WINDOW_CLASS_NAME[] = L"SYSTEM_OVERWRITE_WINDOW";
constexpr wchar_t WINDOW_TITLE[] = L"SYSTEM::OVERWRITE - OpenGL Debug Console";

HDC gDeviceContext = nullptr;
HGLRC gOpenGlContext = nullptr;
bool gRunning = true;

bool SetupPixelFormat(HDC deviceContext)
{
    PIXELFORMATDESCRIPTOR descriptor = {};
    descriptor.nSize = sizeof(descriptor);
    descriptor.nVersion = 1;
    descriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    descriptor.iPixelType = PFD_TYPE_RGBA;
    descriptor.cColorBits = 32;
    descriptor.cDepthBits = 24;
    descriptor.cStencilBits = 8;
    descriptor.iLayerType = PFD_MAIN_PLANE;

    const int pixelFormat = ChoosePixelFormat(deviceContext, &descriptor);
    if (pixelFormat == 0)
    {
        return false;
    }

    return SetPixelFormat(deviceContext, pixelFormat, &descriptor) == TRUE;
}

bool CreateOpenGlContext(HWND window)
{
    gDeviceContext = GetDC(window);
    if (gDeviceContext == nullptr)
    {
        return false;
    }

    if (!SetupPixelFormat(gDeviceContext))
    {
        return false;
    }

    gOpenGlContext = wglCreateContext(gDeviceContext);
    if (gOpenGlContext == nullptr)
    {
        return false;
    }

    return wglMakeCurrent(gDeviceContext, gOpenGlContext) == TRUE;
}

void DestroyOpenGlContext(HWND window)
{
    if (gOpenGlContext != nullptr)
    {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(gOpenGlContext);
        gOpenGlContext = nullptr;
    }

    if (gDeviceContext != nullptr)
    {
        ReleaseDC(window, gDeviceContext);
        gDeviceContext = nullptr;
    }
}

void ResizeViewport(int width, int height)
{
    if (height <= 0)
    {
        height = 1;
    }

    glViewport(0, 0, width, height);
}

void RenderFrame()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    SwapBuffers(gDeviceContext);
}

LRESULT CALLBACK WindowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        if (!CreateOpenGlContext(window))
        {
            MessageBoxW(window, L"OpenGL context creation failed.", WINDOW_TITLE, MB_OK | MB_ICONERROR);
            return -1;
        }
        return 0;

    case WM_SIZE:
        ResizeViewport(LOWORD(lParam), HIWORD(lParam));
        return 0;

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
        {
            DestroyWindow(window);
            return 0;
        }
        break;

    case WM_CLOSE:
        DestroyWindow(window);
        return 0;

    case WM_DESTROY:
        DestroyOpenGlContext(window);
        gRunning = false;
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(window, message, wParam, lParam);
}

bool RegisterMainWindowClass(HINSTANCE instance)
{
    WNDCLASSEXW windowClass = {};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProcedure;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.lpszClassName = WINDOW_CLASS_NAME;

    return RegisterClassExW(&windowClass) != 0;
}
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand)
{
    if (!RegisterMainWindowClass(instance))
    {
        MessageBoxW(nullptr, L"Window class registration failed.", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return 1;
    }

    HWND window = CreateWindowExW(
        0,
        WINDOW_CLASS_NAME,
        WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1280,
        720,
        nullptr,
        nullptr,
        instance,
        nullptr);

    if (window == nullptr)
    {
        MessageBoxW(nullptr, L"Window creation failed.", WINDOW_TITLE, MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(window, showCommand);
    UpdateWindow(window);

    MSG message = {};
    while (gRunning)
    {
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
        {
            if (message.message == WM_QUIT)
            {
                gRunning = false;
                break;
            }

            TranslateMessage(&message);
            DispatchMessageW(&message);
        }

        if (gRunning)
        {
            RenderFrame();
        }
    }

    return static_cast<int>(message.wParam);
}
