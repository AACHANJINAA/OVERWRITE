#pragma once

#include "Component.h"

class CAMERACOMPONENT final : public COMPONENT
{
public:
    void begin_drag(int _mouse_x, int _mouse_y);
    void update_drag(int _mouse_x, int _mouse_y);
    void end_drag();
    void apply_zoom(int _wheel_delta);
    void apply_view() const;

private:
    int last_mouse_x = 0;
    int last_mouse_y = 0;
    bool is_dragging = false;
    float rotation_x = 24.0f;
    float rotation_y = -24.0f;
    float zoom_distance = 10.5f;
    float drag_sensitivity = 0.35f;
    float zoom_sensitivity = 0.7f;
};
