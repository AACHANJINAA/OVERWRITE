#pragma once

#include "Manager.h"
#include "OpenGlRenderComponent.h"
#include "stdafx.h"

class WINDOWMANAGER final : public MANAGER<WINDOWMANAGER>
{
public:
    bool initialize(HINSTANCE _instance, int _show_command);
    int run();
    LRESULT handle_message(UINT _message, WPARAM _w_param, LPARAM _l_param);

private:
    friend class MANAGER<WINDOWMANAGER>;

    WINDOWMANAGER() = default;

    bool register_window_class();
    static LRESULT CALLBACK window_procedure(HWND _window, UINT _message, WPARAM _w_param, LPARAM _l_param);

    HINSTANCE instance = nullptr;
    HWND window = nullptr;
    bool is_running = true;
    OPENGLRENDERCOMPONENT render_component;
};
