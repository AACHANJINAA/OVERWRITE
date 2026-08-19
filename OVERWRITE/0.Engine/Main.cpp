#include "stdafx.h"
#include "WindowManager.h"

int WINAPI wWinMain(HINSTANCE _instance, HINSTANCE, PWSTR, int _show_command)
{
    WINDOWMANAGER& _window_manager = WINDOWMANAGER::get_instance();
    if (!_window_manager.initialize(_instance, _show_command))
    {
        return 1;
    }

    return _window_manager.run();
}
