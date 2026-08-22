#include "stdafx.h"
#include "CJ_MathHelper.h"
#include "CameraComponent.h"

void CAMERACOMPONENT::begin_drag(int _mouse_x, int _mouse_y)
{
    is_dragging = true;
    last_mouse_x = _mouse_x;
    last_mouse_y = _mouse_y;
}

void CAMERACOMPONENT::update_drag(int _mouse_x, int _mouse_y)
{
    if (!is_dragging)
    {
        return;
    }

    const int _delta_x = _mouse_x - last_mouse_x;
    const int _delta_y = _mouse_y - last_mouse_y;

    rotation_y += static_cast<float>(_delta_x) * drag_sensitivity;
    rotation_x += static_cast<float>(_delta_y) * drag_sensitivity;
    rotation_x = cj_math_helper::clamp(rotation_x, -10.0f, 75.0f);

    last_mouse_x = _mouse_x;
    last_mouse_y = _mouse_y;
}

void CAMERACOMPONENT::end_drag()
{
    is_dragging = false;
}

void CAMERACOMPONENT::apply_zoom(int _wheel_delta)
{
    const float _direction = _wheel_delta > 0 ? -1.0f : 1.0f;
    zoom_distance += _direction * zoom_sensitivity;
    zoom_distance = cj_math_helper::clamp(zoom_distance, 6.5f, 18.0f);
}

void CAMERACOMPONENT::apply_view() const
{
    glTranslatef(0.0f, -0.55f, -zoom_distance);
    glRotatef(rotation_x, 1.0f, 0.0f, 0.0f);
    glRotatef(rotation_y, 0.0f, 1.0f, 0.0f);
}
