#include "stdafx.h"
#include "CJ_MathHelper.h"
#include "OpenGlRenderComponent.h"

bool OPENGLRENDERCOMPONENT::initialize(HWND _window)
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

    if (wglMakeCurrent(device_context, open_gl_context) != TRUE)
    {
        shutdown();
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_FLAT);
    glClearDepth(1.0);
    glLineWidth(1.5f);
    resize_viewport(viewport_width, viewport_height);
    return true;
}

void OPENGLRENDERCOMPONENT::shutdown()
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

void OPENGLRENDERCOMPONENT::resize_viewport(int _width, int _height)
{
    if (_height <= 0)
    {
        _height = 1;
    }

    viewport_width = _width;
    viewport_height = _height;
    glViewport(0, 0, _width, _height);
    update_projection();
}

void OPENGLRENDERCOMPONENT::render_frame()
{
    glClearColor(0.005f, 0.008f, 0.014f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    camera_component.apply_view();

    draw_floor_grid();
    draw_console_board();
    draw_circuit_lines();
    draw_stl_nodes();
    draw_helmet_overlay();

    SwapBuffers(device_context);
}

void OPENGLRENDERCOMPONENT::begin_camera_drag(int _mouse_x, int _mouse_y)
{
    camera_component.begin_drag(_mouse_x, _mouse_y);
}

void OPENGLRENDERCOMPONENT::update_camera_drag(int _mouse_x, int _mouse_y)
{
    camera_component.update_drag(_mouse_x, _mouse_y);
}

void OPENGLRENDERCOMPONENT::end_camera_drag()
{
    camera_component.end_drag();
}

void OPENGLRENDERCOMPONENT::zoom_camera(int _wheel_delta)
{
    camera_component.apply_zoom(_wheel_delta);
}

bool OPENGLRENDERCOMPONENT::setup_pixel_format(HDC _device_context)
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

void OPENGLRENDERCOMPONENT::update_projection() const
{
    const float _aspect = cj_math_helper::safe_aspect(viewport_width, viewport_height);
    const double _near_plane = 1.0;
    const double _far_plane = 64.0;
    const double _top = 0.42;
    const double _right = _top * static_cast<double>(_aspect);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-_right, _right, -_top, _top, _near_plane, _far_plane);
    glMatrixMode(GL_MODELVIEW);
}

void OPENGLRENDERCOMPONENT::set_color(const COLORRGB& _color) const
{
    glColor3f(_color.red, _color.green, _color.blue);
}

void OPENGLRENDERCOMPONENT::draw_line(const VECTOR3& _start, const VECTOR3& _end, const COLORRGB& _color) const
{
    set_color(_color);
    glBegin(GL_LINES);
    glVertex3f(_start.x, _start.y, _start.z);
    glVertex3f(_end.x, _end.y, _end.z);
    glEnd();
}

void OPENGLRENDERCOMPONENT::draw_floor_grid() const
{
    const COLORRGB _grid_color = { 0.02f, 0.22f, 0.28f };

    glLineWidth(1.0f);
    for (int _index = -10; _index <= 10; ++_index)
    {
        const float _line_position = static_cast<float>(_index) * 0.55f;
        draw_line({ -5.5f, -0.02f, _line_position }, { 5.5f, -0.02f, _line_position }, _grid_color);
        draw_line({ _line_position, -0.02f, -5.5f }, { _line_position, -0.02f, 5.5f }, _grid_color);
    }
}

void OPENGLRENDERCOMPONENT::draw_console_board() const
{
    const COLORRGB _board_color = { 0.0f, 0.86f, 0.95f };
    const COLORRGB _inner_color = { 0.0f, 0.35f, 0.65f };

    glLineWidth(2.5f);
    glBegin(GL_LINE_LOOP);
    set_color(_board_color);
    glVertex3f(-4.0f, 0.0f, -2.4f);
    glVertex3f(4.0f, 0.0f, -2.4f);
    glVertex3f(4.0f, 0.0f, 2.4f);
    glVertex3f(-4.0f, 0.0f, 2.4f);
    glEnd();

    glLineWidth(1.5f);
    glBegin(GL_LINES);
    set_color(_inner_color);
    glVertex3f(-3.2f, 0.02f, -1.6f);
    glVertex3f(3.2f, 0.02f, -1.6f);
    glVertex3f(-3.2f, 0.02f, 1.6f);
    glVertex3f(3.2f, 0.02f, 1.6f);
    glVertex3f(-2.2f, 0.02f, -2.0f);
    glVertex3f(-2.2f, 0.02f, 2.0f);
    glVertex3f(2.2f, 0.02f, -2.0f);
    glVertex3f(2.2f, 0.02f, 2.0f);
    glEnd();
}

void OPENGLRENDERCOMPONENT::draw_circuit_lines() const
{
    const COLORRGB _normal_line_color = { 0.0f, 0.72f, 0.92f };
    const COLORRGB _warning_line_color = { 1.0f, 0.14f, 0.08f };
    const VECTOR3 _hub_position = { 0.0f, 0.08f, 0.0f };
    const STLNODE* _nodes = get_stl_nodes();

    glLineWidth(2.0f);
    for (int _index = 0; _index < stl_node_count; ++_index)
    {
        const STLNODE& _node = _nodes[_index];
        const COLORRGB _line_color = _node.is_bugged ? _warning_line_color : _normal_line_color;
        draw_line(_hub_position, { _node.position.x, 0.08f, _node.position.z }, _line_color);
    }

    draw_line({ -3.6f, 0.08f, -2.1f }, { -2.5f, 0.08f, -1.35f }, _warning_line_color);
    draw_line({ 3.6f, 0.08f, 2.1f }, { 2.45f, 0.08f, 1.25f }, _normal_line_color);
}

void OPENGLRENDERCOMPONENT::draw_stl_nodes() const
{
    const STLNODE* _nodes = get_stl_nodes();

    for (int _index = 0; _index < stl_node_count; ++_index)
    {
        const STLNODE& _node = _nodes[_index];
        const COLORRGB _node_color = _node.is_bugged ? COLORRGB{ 1.0f, 0.08f, 0.06f } : COLORRGB{ 0.0f, 0.78f, 1.0f };
        const float _node_size = _node.is_bugged ? 0.42f : 0.34f;
        draw_wire_cube(_node.position, _node_size, _node_color);
    }
}

void OPENGLRENDERCOMPONENT::draw_wire_cube(const VECTOR3& _center, float _size, const COLORRGB& _color) const
{
    const float _half_size = _size * 0.5f;
    const float _left = _center.x - _half_size;
    const float _right = _center.x + _half_size;
    const float _bottom = _center.y - _half_size;
    const float _top = _center.y + _half_size;
    const float _back = _center.z - _half_size;
    const float _front = _center.z + _half_size;

    glLineWidth(2.5f);
    set_color(_color);
    glBegin(GL_LINES);
    glVertex3f(_left, _bottom, _back);
    glVertex3f(_right, _bottom, _back);
    glVertex3f(_right, _bottom, _back);
    glVertex3f(_right, _bottom, _front);
    glVertex3f(_right, _bottom, _front);
    glVertex3f(_left, _bottom, _front);
    glVertex3f(_left, _bottom, _front);
    glVertex3f(_left, _bottom, _back);

    glVertex3f(_left, _top, _back);
    glVertex3f(_right, _top, _back);
    glVertex3f(_right, _top, _back);
    glVertex3f(_right, _top, _front);
    glVertex3f(_right, _top, _front);
    glVertex3f(_left, _top, _front);
    glVertex3f(_left, _top, _front);
    glVertex3f(_left, _top, _back);

    glVertex3f(_left, _bottom, _back);
    glVertex3f(_left, _top, _back);
    glVertex3f(_right, _bottom, _back);
    glVertex3f(_right, _top, _back);
    glVertex3f(_right, _bottom, _front);
    glVertex3f(_right, _top, _front);
    glVertex3f(_left, _bottom, _front);
    glVertex3f(_left, _top, _front);
    glEnd();
}

void OPENGLRENDERCOMPONENT::draw_helmet_overlay() const
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, static_cast<double>(viewport_width), static_cast<double>(viewport_height), 0.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const float _left = static_cast<float>(viewport_width) * 0.08f;
    const float _right = static_cast<float>(viewport_width) * 0.92f;
    const float _top = static_cast<float>(viewport_height) * 0.10f;
    const float _bottom = static_cast<float>(viewport_height) * 0.92f;
    const float _center_x = static_cast<float>(viewport_width) * 0.5f;
    const float _center_y = static_cast<float>(viewport_height) * 0.5f;

    glBegin(GL_QUADS);
    glColor4f(0.0f, 0.12f, 0.16f, 0.24f);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(_left, 0.0f);
    glVertex2f(_left * 0.72f, static_cast<float>(viewport_height));
    glVertex2f(0.0f, static_cast<float>(viewport_height));

    glVertex2f(_right, 0.0f);
    glVertex2f(static_cast<float>(viewport_width), 0.0f);
    glVertex2f(static_cast<float>(viewport_width), static_cast<float>(viewport_height));
    glVertex2f(_right * 1.03f, static_cast<float>(viewport_height));

    glColor4f(0.0f, 0.08f, 0.10f, 0.16f);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(static_cast<float>(viewport_width), 0.0f);
    glVertex2f(static_cast<float>(viewport_width), _top * 0.55f);
    glVertex2f(0.0f, _top * 0.75f);

    glVertex2f(0.0f, _bottom);
    glVertex2f(static_cast<float>(viewport_width), _bottom * 0.98f);
    glVertex2f(static_cast<float>(viewport_width), static_cast<float>(viewport_height));
    glVertex2f(0.0f, static_cast<float>(viewport_height));
    glEnd();

    glLineWidth(1.0f);
    glColor4f(0.76f, 0.96f, 1.0f, 0.20f);
    glBegin(GL_LINE_STRIP);
    glVertex2f(_left * 0.95f, _center_y * 0.76f);
    glVertex2f(_left * 1.42f, _top * 1.34f);
    glVertex2f(_center_x * 0.82f, _top * 0.82f);
    glVertex2f(_center_x * 1.18f, _top * 0.82f);
    glVertex2f(_right * 0.88f, _top * 1.34f);
    glVertex2f(_right * 1.01f, _center_y * 0.76f);
    glEnd();

    glColor4f(0.0f, 0.48f, 0.58f, 0.16f);
    glBegin(GL_LINE_STRIP);
    glVertex2f(_left * 0.92f, _center_y * 0.86f);
    glVertex2f(_left * 0.88f, _bottom * 0.76f);
    glVertex2f(_left * 1.10f, _bottom * 0.96f);
    glVertex2f(_center_x, _bottom * 0.99f);
    glVertex2f(_right * 0.95f, _bottom * 0.96f);
    glVertex2f(_right * 1.03f, _bottom * 0.76f);
    glVertex2f(_right * 1.02f, _center_y * 0.86f);
    glEnd();

    glLineWidth(1.4f);
    glColor4f(0.95f, 1.0f, 1.0f, 0.30f);
    glBegin(GL_LINES);
    glVertex2f(_left * 1.38f, _top * 1.15f);
    glVertex2f(_left * 2.45f, _top * 0.82f);
    glVertex2f(_left * 1.22f, _top * 1.55f);
    glVertex2f(_left * 1.86f, _top * 1.36f);
    glEnd();

    glLineWidth(0.75f);
    glColor4f(0.68f, 0.94f, 1.0f, 0.10f);
    glBegin(GL_LINES);
    glVertex2f(_left * 1.50f, _center_y * 0.46f);
    glVertex2f(_left * 2.80f, _center_y * 0.40f);
    glVertex2f(_right * 0.78f, _center_y * 0.44f);
    glVertex2f(_right * 0.95f, _center_y * 0.49f);
    glVertex2f(_left * 1.45f, _bottom * 0.84f);
    glVertex2f(_left * 2.75f, _bottom * 0.80f);
    glVertex2f(_right * 0.80f, _bottom * 0.80f);
    glVertex2f(_right * 0.96f, _bottom * 0.84f);
    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

const STLNODE* OPENGLRENDERCOMPONENT::get_stl_nodes() const
{
    static const STLNODE _nodes[] =
    {
        { L"Warp Engine Control", { -2.5f, 0.35f, -1.35f }, true },
        { L"Auto Turret Target Array", { 2.35f, 0.35f, -1.15f }, true },
        { L"Long Range Radar Lookup", { 0.25f, 0.35f, 1.55f }, true },
        { L"Life Support Cache", { -2.0f, 0.25f, 1.15f }, false },
        { L"Shield Event Queue", { 2.45f, 0.25f, 1.25f }, false },
    };

    return _nodes;
}
