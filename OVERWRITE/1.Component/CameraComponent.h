#pragma once

#include "Component.h"
#include "RenderTypes.h"

class CAMERACOMPONENT final : public COMPONENT
{
public:
    void begin_look_drag(int _mouse_x, int _mouse_y);
    void update_look_drag(int _mouse_x, int _mouse_y);
    void end_look_drag();
    void set_move_forward(bool _is_pressed);
    void set_move_backward(bool _is_pressed);
    void set_move_left(bool _is_pressed);
    void set_move_right(bool _is_pressed);
    void update_movement();
    void apply_view() const;

private:
    int last_mouse_x = 0;
    int last_mouse_y = 0;
    bool is_looking = false;
    bool is_moving_forward = false;
    bool is_moving_backward = false;
    bool is_moving_left = false;
    bool is_moving_right = false;
    VECTOR3 position = { 0.0f, 1.35f, 4.8f };
    float pitch = 0.0f;
    float yaw = 0.0f;
    float look_sensitivity = 0.18f;
    float move_speed = 0.075f;
};
