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
    void begin_view_drag(int _mouse_x, int _mouse_y);
    void update_view_drag(int _mouse_x, int _mouse_y);
    void end_view_drag();
    void set_move_forward(bool _is_pressed);
    void set_move_backward(bool _is_pressed);
    void set_move_left(bool _is_pressed);
    void set_move_right(bool _is_pressed);
    void interact();
    void puzzle_click(int _mouse_x, int _mouse_y);

private:
    bool setup_pixel_format(HDC _device_context);
    bool initialize_shader_program();
    void shutdown_shader_program();
    void update_projection() const;
    void set_color(const COLORRGB& _color) const;
    void draw_line(const VECTOR3& _start, const VECTOR3& _end, const COLORRGB& _color) const;
    void draw_spaceship_interior() const;
    void draw_window_section() const;
    void draw_repair_console() const;
    void draw_wall_panels() const;
    void draw_floor_details() const;
    void draw_ceiling_details() const;
    void draw_wall_module(float _z) const;
    void draw_earth_preview() const;
    void draw_stl_nodes() const;
    void draw_wire_cube(const VECTOR3& _center, float _size, const COLORRGB& _color) const;
    void draw_shader_glass_overlay() const;
    void draw_helmet_overlay() const;
    const STLNODE* get_stl_nodes() const;

    static constexpr int stl_node_count = 5;
    HWND window = nullptr;
    HDC device_context = nullptr;
    HGLRC open_gl_context = nullptr;
    GLuint glass_shader_program = 0;
    CAMERACOMPONENT camera_component;
    int viewport_width = 1280;
    int viewport_height = 720;
};
