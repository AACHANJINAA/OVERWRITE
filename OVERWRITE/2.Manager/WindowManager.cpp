#include "stdafx.h"
#include "WindowManager.h"

namespace
{
constexpr wchar_t _window_class_name[] = L"SYSTEM_OVERWRITE_WINDOW";
constexpr wchar_t _window_title[] = L"SYSTEM::OVERWRITE - OpenGL Debug Console";
}

bool WINDOWMANAGER::initialize(HINSTANCE _instance, int _show_command)
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

int WINDOWMANAGER::run()
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

LRESULT WINDOWMANAGER::handle_message(UINT _message, WPARAM _w_param, LPARAM _l_param)
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

    case WM_LBUTTONDOWN:
        SetCapture(window);
        render_component.begin_camera_drag(GET_X_LPARAM(_l_param), GET_Y_LPARAM(_l_param));
        return 0;

    case WM_MOUSEMOVE:
        if ((_w_param & MK_LBUTTON) == MK_LBUTTON)
        {
            render_component.update_camera_drag(GET_X_LPARAM(_l_param), GET_Y_LPARAM(_l_param));
            return 0;
        }
        break;

    case WM_LBUTTONUP:
        render_component.end_camera_drag();
        ReleaseCapture();
        return 0;

    case WM_MOUSEWHEEL:
        render_component.zoom_camera(GET_WHEEL_DELTA_WPARAM(_w_param));
        return 0;

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

bool WINDOWMANAGER::register_window_class()
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

LRESULT CALLBACK WINDOWMANAGER::window_procedure(HWND _window, UINT _message, WPARAM _w_param, LPARAM _l_param)
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
