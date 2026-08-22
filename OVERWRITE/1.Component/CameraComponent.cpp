#include "stdafx.h"
#include "CJ_MathHelper.h"
#include "CameraComponent.h"

void CAMERACOMPONENT::begin_look_drag(int _mouse_x, int _mouse_y)
{
    is_looking = true;
    last_mouse_x = _mouse_x;
    last_mouse_y = _mouse_y;
}

void CAMERACOMPONENT::update_look_drag(int _mouse_x, int _mouse_y)
{
    if (!is_looking)
    {
        return;
    }

    const int _delta_x = _mouse_x - last_mouse_x;
    const int _delta_y = _mouse_y - last_mouse_y;

    yaw -= static_cast<float>(_delta_x) * look_sensitivity;
    pitch -= static_cast<float>(_delta_y) * look_sensitivity;
    pitch = cj_math_helper::clamp(pitch, -70.0f, 70.0f);

    last_mouse_x = _mouse_x;
    last_mouse_y = _mouse_y;
}

void CAMERACOMPONENT::end_look_drag()
{
    is_looking = false;
}

void CAMERACOMPONENT::set_move_forward(bool _is_pressed)
{
    is_moving_forward = _is_pressed;
}

void CAMERACOMPONENT::set_move_backward(bool _is_pressed)
{
    is_moving_backward = _is_pressed;
}

void CAMERACOMPONENT::set_move_left(bool _is_pressed)
{
    is_moving_left = _is_pressed;
}

void CAMERACOMPONENT::set_move_right(bool _is_pressed)
{
    is_moving_right = _is_pressed;
}

void CAMERACOMPONENT::update_movement()
{
    const float _yaw_radian = cj_math_helper::degree_to_radian(yaw);
    const float _pitch_radian = cj_math_helper::degree_to_radian(pitch);
    const float _pitch_flatten = std::cos(_pitch_radian);
    float _forward_x = std::sin(_yaw_radian) * _pitch_flatten;
    float _forward_z = -std::cos(_yaw_radian) * _pitch_flatten;
    const float _forward_length = std::sqrt((_forward_x * _forward_x) + (_forward_z * _forward_z));

    if (_forward_length > 0.0001f)
    {
        _forward_x /= _forward_length;
        _forward_z /= _forward_length;
    }
    else
    {
        _forward_x = std::sin(_yaw_radian);
        _forward_z = -std::cos(_yaw_radian);
    }

    const float _right_x = -_forward_z;
    const float _right_z = _forward_x;
    float _move_x = 0.0f;
    float _move_z = 0.0f;

    if (is_moving_forward)
    {
        _move_x += _forward_x;
        _move_z += _forward_z;
    }

    if (is_moving_backward)
    {
        _move_x -= _forward_x;
        _move_z -= _forward_z;
    }

    if (is_moving_right)
    {
        _move_x += _right_x;
        _move_z += _right_z;
    }

    if (is_moving_left)
    {
        _move_x -= _right_x;
        _move_z -= _right_z;
    }

    const float _move_length = std::sqrt((_move_x * _move_x) + (_move_z * _move_z));
    if (_move_length > 0.0001f)
    {
        _move_x /= _move_length;
        _move_z /= _move_length;
    }

    position.x += _move_x * move_speed;
    position.z += _move_z * move_speed;
    position.x = cj_math_helper::clamp(position.x, -3.2f, 3.2f);
    position.z = cj_math_helper::clamp(position.z, -5.8f, 5.2f);
}

void CAMERACOMPONENT::apply_view() const
{
    glRotatef(-pitch, 1.0f, 0.0f, 0.0f);
    glRotatef(-yaw, 0.0f, 1.0f, 0.0f);
    glTranslatef(-position.x, -position.y, -position.z);
}
