#pragma once

#include "Component.h"
#include "CameraComponent.h"
#include "RenderTypes.h"
#include "stdafx.h"

class OPENGLRENDERCOMPONENT final : public COMPONENT
{
public:
    bool initialize(HWND _window);
    void shutdown();
    void resize_viewport(int _width, int _height);
    void render_frame();
    void begin_camera_drag(int _mouse_x, int _mouse_y);
    void update_camera_drag(int _mouse_x, int _mouse_y);
    void end_camera_drag();
    void zoom_camera(int _wheel_delta);

private:
    bool setup_pixel_format(HDC _device_context);
    void update_projection() const;
    void set_color(const COLORRGB& _color) const;
    void draw_line(const VECTOR3& _start, const VECTOR3& _end, const COLORRGB& _color) const;
    void draw_floor_grid() const;
    void draw_console_board() const;
    void draw_circuit_lines() const;
    void draw_stl_nodes() const;
    void draw_wire_cube(const VECTOR3& _center, float _size, const COLORRGB& _color) const;
    void draw_helmet_overlay() const;
    const STLNODE* get_stl_nodes() const;

    static constexpr int stl_node_count = 5;
    HWND window = nullptr;
    HDC device_context = nullptr;
    HGLRC open_gl_context = nullptr;
    CAMERACOMPONENT camera_component;
    int viewport_width = 1280;
    int viewport_height = 720;
};
