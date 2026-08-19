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
    glTranslatef(0.0f, -0.55f, -10.5f);
    glRotatef(24.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(-24.0f, 0.0f, 1.0f, 0.0f);

    draw_floor_grid();
    draw_console_board();
    draw_circuit_lines();
    draw_stl_nodes();

    SwapBuffers(device_context);
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
