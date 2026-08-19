#include <windows.h>
#include <gl/GL.h>

namespace
{
constexpr wchar_t _window_class_name[] = L"SYSTEM_OVERWRITE_WINDOW";
constexpr wchar_t _window_title[] = L"SYSTEM::OVERWRITE - OpenGL Debug Console";

class OPENGLRENDERCOMPONENT
{
public:
    bool initialize(HWND _window)
    {
        window = _window;
        device_context = GetDC(window);
        if (device_context == nullptr)
        {
            return false;
        }

        if (!setup_pixel_format(device_context))
        {
            shutdown();
            return false;
        }

        open_gl_context = wglCreateContext(device_context);
        if (open_gl_context == nullptr)
        {
            shutdown();
            return false;
        }

        return wglMakeCurrent(device_context, open_gl_context) == TRUE;
    }

    void shutdown()
    {
        if (open_gl_context != nullptr)
        {
            wglMakeCurrent(nullptr, nullptr);
            wglDeleteContext(open_gl_context);
            open_gl_context = nullptr;
        }

        if (device_context != nullptr)
        {
            ReleaseDC(window, device_context);
            device_context = nullptr;
        }

        window = nullptr;
    }

    void resize_viewport(int _width, int _height)
    {
        if (_height <= 0)
        {
            _height = 1;
        }

        glViewport(0, 0, _width, _height);
    }

    void render_frame() const
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        SwapBuffers(device_context);
    }

private:
    bool setup_pixel_format(HDC _device_context)
    {
        PIXELFORMATDESCRIPTOR _descriptor = {};
        _descriptor.nSize = sizeof(_descriptor);
        _descriptor.nVersion = 1;
        _descriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        _descriptor.iPixelType = PFD_TYPE_RGBA;
        _descriptor.cColorBits = 32;
        _descriptor.cDepthBits = 24;
        _descriptor.cStencilBits = 8;
        _descriptor.iLayerType = PFD_MAIN_PLANE;

        const int _pixel_format = ChoosePixelFormat(_device_context, &_descriptor);
        if (_pixel_format == 0)
        {
            return false;
        }

        return SetPixelFormat(_device_context, _pixel_format, &_descriptor) == TRUE;
    }

    HWND window = nullptr;
    HDC device_context = nullptr;
    HGLRC open_gl_context = nullptr;
};

class WINDOWMANAGER
{
public:
    bool initialize(HINSTANCE _instance, int _show_command)
    {
        instance = _instance;
        if (!register_window_class())
        {
            MessageBoxW(nullptr, L"Window class registration failed.", _window_title, MB_OK | MB_ICONERROR);
            return false;
        }

        window = CreateWindowExW(
            0,
            _window_class_name,
            _window_title,
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            1280,
            720,
            nullptr,
            nullptr,
            instance,
            this);

        if (window == nullptr)
        {
            MessageBoxW(nullptr, L"Window creation failed.", _window_title, MB_OK | MB_ICONERROR);
            return false;
        }

        ShowWindow(window, _show_command);
        UpdateWindow(window);
        return true;
    }

    int run()
    {
        MSG _message = {};
        while (is_running)
        {
            while (PeekMessageW(&_message, nullptr, 0, 0, PM_REMOVE))
            {
                if (_message.message == WM_QUIT)
                {
                    is_running = false;
                    break;
                }

                TranslateMessage(&_message);
                DispatchMessageW(&_message);
            }

            if (is_running)
            {
                render_component.render_frame();
            }
        }

        return static_cast<int>(_message.wParam);
    }

    LRESULT handle_message(UINT _message, WPARAM _w_param, LPARAM _l_param)
    {
        switch (_message)
        {
        case WM_CREATE:
            if (!render_component.initialize(window))
            {
                MessageBoxW(window, L"OpenGL context creation failed.", _window_title, MB_OK | MB_ICONERROR);
                return -1;
            }
            return 0;

        case WM_SIZE:
            render_component.resize_viewport(LOWORD(_l_param), HIWORD(_l_param));
            return 0;

        case WM_KEYDOWN:
            if (_w_param == VK_ESCAPE)
            {
                DestroyWindow(window);
                return 0;
            }
            break;

        case WM_CLOSE:
            DestroyWindow(window);
            return 0;

        case WM_DESTROY:
            render_component.shutdown();
            is_running = false;
            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProcW(window, _message, _w_param, _l_param);
    }

private:
    bool register_window_class()
    {
        WNDCLASSEXW _window_class = {};
        _window_class.cbSize = sizeof(_window_class);
        _window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
        _window_class.lpfnWndProc = window_procedure;
        _window_class.hInstance = instance;
        _window_class.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        _window_class.lpszClassName = _window_class_name;

        return RegisterClassExW(&_window_class) != 0;
    }

    static LRESULT CALLBACK window_procedure(HWND _window, UINT _message, WPARAM _w_param, LPARAM _l_param)
    {
        WINDOWMANAGER* _window_manager = nullptr;

        if (_message == WM_NCCREATE)
        {
            CREATESTRUCTW* _create_struct = reinterpret_cast<CREATESTRUCTW*>(_l_param);
            _window_manager = reinterpret_cast<WINDOWMANAGER*>(_create_struct->lpCreateParams);
            _window_manager->window = _window;
            SetWindowLongPtrW(_window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(_window_manager));
        }
        else
        {
            _window_manager = reinterpret_cast<WINDOWMANAGER*>(GetWindowLongPtrW(_window, GWLP_USERDATA));
        }

        if (_window_manager != nullptr)
        {
            return _window_manager->handle_message(_message, _w_param, _l_param);
        }

        return DefWindowProcW(_window, _message, _w_param, _l_param);
    }

    HINSTANCE instance = nullptr;
    HWND window = nullptr;
    bool is_running = true;
    OPENGLRENDERCOMPONENT render_component;
};
}

int WINAPI wWinMain(HINSTANCE _instance, HINSTANCE, PWSTR, int _show_command)
{
    WINDOWMANAGER _window_manager;
    if (!_window_manager.initialize(_instance, _show_command))
    {
        return 1;
    }

    return _window_manager.run();
}
