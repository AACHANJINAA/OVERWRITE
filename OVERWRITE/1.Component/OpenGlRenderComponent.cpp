#include "stdafx.h"
#include "CJ_MathHelper.h"
#include "OpenGlRenderComponent.h"

namespace
{
constexpr GLenum _gl_vertex_shader = 0x8B31;
constexpr GLenum _gl_fragment_shader = 0x8B30;
constexpr GLenum _gl_compile_status = 0x8B81;
constexpr GLenum _gl_link_status = 0x8B82;

using GL_CREATE_SHADER = GLuint(APIENTRY*)(GLenum);
using GL_SHADER_SOURCE = void(APIENTRY*)(GLuint, GLsizei, const char* const*, const GLint*);
using GL_COMPILE_SHADER = void(APIENTRY*)(GLuint);
using GL_GET_SHADER_IV = void(APIENTRY*)(GLuint, GLenum, GLint*);
using GL_DELETE_SHADER = void(APIENTRY*)(GLuint);
using GL_CREATE_PROGRAM = GLuint(APIENTRY*)();
using GL_ATTACH_SHADER = void(APIENTRY*)(GLuint, GLuint);
using GL_LINK_PROGRAM = void(APIENTRY*)(GLuint);
using GL_GET_PROGRAM_IV = void(APIENTRY*)(GLuint, GLenum, GLint*);
using GL_USE_PROGRAM = void(APIENTRY*)(GLuint);
using GL_DELETE_PROGRAM = void(APIENTRY*)(GLuint);
using GL_GET_UNIFORM_LOCATION = GLint(APIENTRY*)(GLuint, const char*);
using GL_UNIFORM_1F = void(APIENTRY*)(GLint, GLfloat);
using GL_UNIFORM_2F = void(APIENTRY*)(GLint, GLfloat, GLfloat);

GL_CREATE_SHADER _gl_create_shader = nullptr;
GL_SHADER_SOURCE _gl_shader_source = nullptr;
GL_COMPILE_SHADER _gl_compile_shader = nullptr;
GL_GET_SHADER_IV _gl_get_shader_iv = nullptr;
GL_DELETE_SHADER _gl_delete_shader = nullptr;
GL_CREATE_PROGRAM _gl_create_program = nullptr;
GL_ATTACH_SHADER _gl_attach_shader = nullptr;
GL_LINK_PROGRAM _gl_link_program = nullptr;
GL_GET_PROGRAM_IV _gl_get_program_iv = nullptr;
GL_USE_PROGRAM _gl_use_program = nullptr;
GL_DELETE_PROGRAM _gl_delete_program = nullptr;
GL_GET_UNIFORM_LOCATION _gl_get_uniform_location = nullptr;
GL_UNIFORM_1F _gl_uniform_1f = nullptr;
GL_UNIFORM_2F _gl_uniform_2f = nullptr;

template <typename FUNCTION>
FUNCTION load_open_gl_function(const char* _name)
{
    return reinterpret_cast<FUNCTION>(wglGetProcAddress(_name));
}

bool load_shader_functions()
{
    _gl_create_shader = load_open_gl_function<GL_CREATE_SHADER>("glCreateShader");
    _gl_shader_source = load_open_gl_function<GL_SHADER_SOURCE>("glShaderSource");
    _gl_compile_shader = load_open_gl_function<GL_COMPILE_SHADER>("glCompileShader");
    _gl_get_shader_iv = load_open_gl_function<GL_GET_SHADER_IV>("glGetShaderiv");
    _gl_delete_shader = load_open_gl_function<GL_DELETE_SHADER>("glDeleteShader");
    _gl_create_program = load_open_gl_function<GL_CREATE_PROGRAM>("glCreateProgram");
    _gl_attach_shader = load_open_gl_function<GL_ATTACH_SHADER>("glAttachShader");
    _gl_link_program = load_open_gl_function<GL_LINK_PROGRAM>("glLinkProgram");
    _gl_get_program_iv = load_open_gl_function<GL_GET_PROGRAM_IV>("glGetProgramiv");
    _gl_use_program = load_open_gl_function<GL_USE_PROGRAM>("glUseProgram");
    _gl_delete_program = load_open_gl_function<GL_DELETE_PROGRAM>("glDeleteProgram");
    _gl_get_uniform_location = load_open_gl_function<GL_GET_UNIFORM_LOCATION>("glGetUniformLocation");
    _gl_uniform_1f = load_open_gl_function<GL_UNIFORM_1F>("glUniform1f");
    _gl_uniform_2f = load_open_gl_function<GL_UNIFORM_2F>("glUniform2f");

    return _gl_create_shader != nullptr
        && _gl_shader_source != nullptr
        && _gl_compile_shader != nullptr
        && _gl_get_shader_iv != nullptr
        && _gl_delete_shader != nullptr
        && _gl_create_program != nullptr
        && _gl_attach_shader != nullptr
        && _gl_link_program != nullptr
        && _gl_get_program_iv != nullptr
        && _gl_use_program != nullptr
        && _gl_delete_program != nullptr
        && _gl_get_uniform_location != nullptr
        && _gl_uniform_1f != nullptr
        && _gl_uniform_2f != nullptr;
}

GLuint compile_shader(GLenum _type, const char* _source)
{
    const GLuint _shader = _gl_create_shader(_type);
    if (_shader == 0)
    {
        return 0;
    }

    _gl_shader_source(_shader, 1, &_source, nullptr);
    _gl_compile_shader(_shader);

    GLint _is_compiled = 0;
    _gl_get_shader_iv(_shader, _gl_compile_status, &_is_compiled);
    if (_is_compiled == 0)
    {
        _gl_delete_shader(_shader);
        return 0;
    }

    return _shader;
}

std::string read_text_file(const char* _file_path)
{
    std::ifstream _file(_file_path);
    if (!_file.is_open())
    {
        return {};
    }

    std::ostringstream _stream;
    _stream << _file.rdbuf();
    return _stream.str();
}
}

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
    initialize_shader_program();
    resize_viewport(viewport_width, viewport_height);
    return true;
}

void OPENGLRENDERCOMPONENT::shutdown()
{
    shutdown_shader_program();

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

    camera_component.update_movement();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    camera_component.apply_view();

    draw_spaceship_interior();
    draw_window_section();
    draw_repair_console();
    draw_wall_panels();
    draw_stl_nodes();
    draw_helmet_overlay();

    SwapBuffers(device_context);
}

void OPENGLRENDERCOMPONENT::begin_view_drag(int _mouse_x, int _mouse_y)
{
    camera_component.begin_look_drag(_mouse_x, _mouse_y);
}

void OPENGLRENDERCOMPONENT::update_view_drag(int _mouse_x, int _mouse_y)
{
    camera_component.update_look_drag(_mouse_x, _mouse_y);
}

void OPENGLRENDERCOMPONENT::end_view_drag()
{
    camera_component.end_look_drag();
}

void OPENGLRENDERCOMPONENT::set_move_forward(bool _is_pressed)
{
    camera_component.set_move_forward(_is_pressed);
}

void OPENGLRENDERCOMPONENT::set_move_backward(bool _is_pressed)
{
    camera_component.set_move_backward(_is_pressed);
}

void OPENGLRENDERCOMPONENT::set_move_left(bool _is_pressed)
{
    camera_component.set_move_left(_is_pressed);
}

void OPENGLRENDERCOMPONENT::set_move_right(bool _is_pressed)
{
    camera_component.set_move_right(_is_pressed);
}

void OPENGLRENDERCOMPONENT::interact()
{
}

void OPENGLRENDERCOMPONENT::puzzle_click(int _mouse_x, int _mouse_y)
{
    UNREFERENCED_PARAMETER(_mouse_x);
    UNREFERENCED_PARAMETER(_mouse_y);
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

bool OPENGLRENDERCOMPONENT::initialize_shader_program()
{
    if (!load_shader_functions())
    {
        return false;
    }

    const std::string _vertex_shader_source = read_text_file("0.Engine/Shader/HelmetGlass.vs");
    const std::string _fragment_shader_source = read_text_file("0.Engine/Shader/HelmetGlass.fs");
    if (_vertex_shader_source.empty() || _fragment_shader_source.empty())
    {
        return false;
    }

    const GLuint _vertex_shader = compile_shader(_gl_vertex_shader, _vertex_shader_source.c_str());
    const GLuint _fragment_shader = compile_shader(_gl_fragment_shader, _fragment_shader_source.c_str());
    if (_vertex_shader == 0 || _fragment_shader == 0)
    {
        if (_vertex_shader != 0)
        {
            _gl_delete_shader(_vertex_shader);
        }

        if (_fragment_shader != 0)
        {
            _gl_delete_shader(_fragment_shader);
        }

        return false;
    }

    const GLuint _program = _gl_create_program();
    _gl_attach_shader(_program, _vertex_shader);
    _gl_attach_shader(_program, _fragment_shader);
    _gl_link_program(_program);

    _gl_delete_shader(_vertex_shader);
    _gl_delete_shader(_fragment_shader);

    GLint _is_linked = 0;
    _gl_get_program_iv(_program, _gl_link_status, &_is_linked);
    if (_is_linked == 0)
    {
        _gl_delete_program(_program);
        return false;
    }

    glass_shader_program = _program;
    return true;
}

void OPENGLRENDERCOMPONENT::shutdown_shader_program()
{
    if (glass_shader_program != 0 && _gl_delete_program != nullptr)
    {
        _gl_delete_program(glass_shader_program);
        glass_shader_program = 0;
    }
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

void OPENGLRENDERCOMPONENT::draw_spaceship_interior() const
{
    const COLORRGB _floor_color = { 0.0f, 0.22f, 0.28f };
    const COLORRGB _rib_color = { 0.0f, 0.48f, 0.58f };
    const COLORRGB _support_color = { 0.0f, 0.62f, 0.72f };
    const float _left = -3.8f;
    const float _right = 3.8f;
    const float _floor = 0.0f;
    const float _ceiling = 3.0f;
    const float _back = 5.8f;
    const float _front = -6.2f;

    glLineWidth(1.0f);
    for (int _index = 0; _index <= 12; ++_index)
    {
        const float _z = _back - static_cast<float>(_index);
        draw_line({ _left, _floor, _z }, { _right, _floor, _z }, _floor_color);
        draw_line({ _left, _ceiling, _z }, { _right, _ceiling, _z }, _floor_color);
        draw_line({ _left, _floor, _z }, { -2.75f, 0.92f, _z }, _rib_color);
        draw_line({ -2.75f, 0.92f, _z }, { -2.45f, 2.35f, _z }, _rib_color);
        draw_line({ -2.45f, 2.35f, _z }, { -1.55f, _ceiling, _z }, _rib_color);
        draw_line({ _right, _floor, _z }, { 2.75f, 0.92f, _z }, _rib_color);
        draw_line({ 2.75f, 0.92f, _z }, { 2.45f, 2.35f, _z }, _rib_color);
        draw_line({ 2.45f, 2.35f, _z }, { 1.55f, _ceiling, _z }, _rib_color);
    }

    for (int _index = -3; _index <= 3; ++_index)
    {
        const float _x = static_cast<float>(_index);
        draw_line({ _x, _floor, _back }, { _x, _floor, _front }, _floor_color);
        draw_line({ _x, _ceiling, _back }, { _x, _ceiling, _front }, _floor_color);
    }

    glLineWidth(2.0f);
    draw_line({ _left, _floor, _back }, { _left, _floor, _front }, _rib_color);
    draw_line({ _right, _floor, _back }, { _right, _floor, _front }, _rib_color);
    draw_line({ -1.55f, _ceiling, _back }, { -1.55f, _ceiling, _front }, _rib_color);
    draw_line({ 1.55f, _ceiling, _back }, { 1.55f, _ceiling, _front }, _rib_color);
    draw_line({ -2.75f, 0.92f, _back }, { -2.75f, 0.92f, _front }, _support_color);
    draw_line({ 2.75f, 0.92f, _back }, { 2.75f, 0.92f, _front }, _support_color);
    draw_line({ -2.45f, 2.35f, _back }, { -2.45f, 2.35f, _front }, _support_color);
    draw_line({ 2.45f, 2.35f, _back }, { 2.45f, 2.35f, _front }, _support_color);

    draw_floor_details();
    draw_ceiling_details();
}

void OPENGLRENDERCOMPONENT::draw_window_section() const
{
    const COLORRGB _window_color = { 0.0f, 0.72f, 0.82f };
    const COLORRGB _dark_glass_color = { 0.0f, 0.16f, 0.20f };
    const float _z = -6.25f;
    const float _glass_z = -6.28f;

    draw_earth_preview();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_POLYGON);
    glColor4f(0.05f, 0.24f, 0.30f, 0.22f);
    glVertex3f(-2.12f, 1.36f, _glass_z);
    glVertex3f(-1.30f, 2.38f, _glass_z);
    glVertex3f(0.0f, 2.64f, _glass_z);
    glVertex3f(1.30f, 2.38f, _glass_z);
    glVertex3f(2.12f, 1.36f, _glass_z);
    glVertex3f(1.48f, 0.82f, _glass_z);
    glVertex3f(-1.48f, 0.82f, _glass_z);
    glEnd();

    glBegin(GL_TRIANGLES);
    glColor4f(0.78f, 0.98f, 1.0f, 0.18f);
    glVertex3f(-1.68f, 2.22f, _glass_z + 0.01f);
    glVertex3f(-0.72f, 2.52f, _glass_z + 0.01f);
    glVertex3f(-1.18f, 1.58f, _glass_z + 0.01f);

    glColor4f(0.78f, 0.98f, 1.0f, 0.10f);
    glVertex3f(0.82f, 2.42f, _glass_z + 0.01f);
    glVertex3f(1.46f, 2.10f, _glass_z + 0.01f);
    glVertex3f(0.58f, 1.24f, _glass_z + 0.01f);
    glEnd();

    glDisable(GL_BLEND);

    glLineWidth(2.5f);
    glBegin(GL_LINE_LOOP);
    set_color(_window_color);
    glVertex3f(-2.2f, 1.35f, _z);
    glVertex3f(-1.35f, 2.45f, _z);
    glVertex3f(0.0f, 2.72f, _z);
    glVertex3f(1.35f, 2.45f, _z);
    glVertex3f(2.2f, 1.35f, _z);
    glVertex3f(1.55f, 0.76f, _z);
    glVertex3f(-1.55f, 0.76f, _z);
    glEnd();

    glLineWidth(1.5f);
    draw_line({ 0.0f, 0.76f, _z }, { 0.0f, 2.72f, _z }, _dark_glass_color);
    draw_line({ -1.55f, 0.76f, _z }, { -1.35f, 2.45f, _z }, _dark_glass_color);
    draw_line({ 1.55f, 0.76f, _z }, { 1.35f, 2.45f, _z }, _dark_glass_color);

    glLineWidth(1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_LINES);
    glColor4f(0.85f, 1.0f, 1.0f, 0.32f);
    glVertex3f(-1.72f, 2.20f, _glass_z + 0.02f);
    glVertex3f(-0.50f, 2.48f, _glass_z + 0.02f);
    glVertex3f(-1.38f, 1.86f, _glass_z + 0.02f);
    glVertex3f(-0.86f, 2.02f, _glass_z + 0.02f);
    glColor4f(0.85f, 1.0f, 1.0f, 0.18f);
    glVertex3f(0.34f, 2.34f, _glass_z + 0.02f);
    glVertex3f(1.32f, 2.02f, _glass_z + 0.02f);
    glEnd();
    glDisable(GL_BLEND);
}

void OPENGLRENDERCOMPONENT::draw_repair_console() const
{
    const COLORRGB _console_color = { 0.0f, 0.78f, 0.92f };
    const COLORRGB _panel_color = { 0.0f, 0.32f, 0.52f };

    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    set_color(_console_color);
    glVertex3f(-1.65f, 0.82f, -2.35f);
    glVertex3f(1.65f, 0.82f, -2.35f);
    glVertex3f(2.05f, 0.42f, -1.25f);
    glVertex3f(-2.05f, 0.42f, -1.25f);
    glEnd();

    glLineWidth(1.0f);
    glBegin(GL_LINES);
    set_color(_panel_color);
    glVertex3f(-1.1f, 0.70f, -2.05f);
    glVertex3f(1.1f, 0.70f, -2.05f);
    glVertex3f(-1.45f, 0.56f, -1.62f);
    glVertex3f(1.45f, 0.56f, -1.62f);
    glVertex3f(-0.72f, 0.78f, -2.25f);
    glVertex3f(-0.92f, 0.47f, -1.34f);
    glVertex3f(0.72f, 0.78f, -2.25f);
    glVertex3f(0.92f, 0.47f, -1.34f);
    glEnd();
}

void OPENGLRENDERCOMPONENT::draw_wall_panels() const
{
    for (int _index = 0; _index < 6; ++_index)
    {
        draw_wall_module(4.85f - static_cast<float>(_index) * 1.85f);
    }
}

void OPENGLRENDERCOMPONENT::draw_floor_details() const
{
    const COLORRGB _grate_color = { 0.0f, 0.36f, 0.44f };
    const COLORRGB _panel_color = { 0.0f, 0.50f, 0.58f };

    glLineWidth(1.0f);
    for (int _index = 0; _index < 13; ++_index)
    {
        const float _z = 5.55f - static_cast<float>(_index) * 0.85f;
        draw_line({ -0.62f, 0.012f, _z }, { 0.62f, 0.012f, _z }, _grate_color);
        draw_line({ -2.65f, 0.014f, _z }, { -1.0f, 0.014f, _z - 0.48f }, _panel_color);
        draw_line({ 2.65f, 0.014f, _z }, { 1.0f, 0.014f, _z - 0.48f }, _panel_color);
    }

    draw_line({ -0.72f, 0.018f, 5.8f }, { -0.72f, 0.018f, -6.2f }, _grate_color);
    draw_line({ 0.72f, 0.018f, 5.8f }, { 0.72f, 0.018f, -6.2f }, _grate_color);
    draw_line({ -2.85f, 0.018f, 5.8f }, { -0.95f, 0.018f, -6.2f }, _panel_color);
    draw_line({ 2.85f, 0.018f, 5.8f }, { 0.95f, 0.018f, -6.2f }, _panel_color);
}

void OPENGLRENDERCOMPONENT::draw_ceiling_details() const
{
    const COLORRGB _panel_color = { 0.0f, 0.30f, 0.38f };
    const COLORRGB _light_color = { 0.46f, 0.96f, 1.0f };

    glLineWidth(1.0f);
    for (int _index = 0; _index < 8; ++_index)
    {
        const float _z = 5.35f - static_cast<float>(_index) * 1.45f;
        draw_line({ -1.15f, 2.985f, _z }, { 1.15f, 2.985f, _z }, _panel_color);
        draw_line({ -1.15f, 2.985f, _z - 0.55f }, { 1.15f, 2.985f, _z - 0.55f }, _panel_color);
        draw_line({ -1.15f, 2.985f, _z }, { -1.15f, 2.985f, _z - 0.55f }, _panel_color);
        draw_line({ 1.15f, 2.985f, _z }, { 1.15f, 2.985f, _z - 0.55f }, _panel_color);

        glLineWidth(2.0f);
        draw_line({ -0.72f, 2.975f, _z - 0.24f }, { 0.72f, 2.975f, _z - 0.24f }, _light_color);
        glLineWidth(1.0f);
    }
}

void OPENGLRENDERCOMPONENT::draw_wall_module(float _z) const
{
    const COLORRGB _panel_color = { 0.0f, 0.30f, 0.38f };
    const COLORRGB _trim_color = { 0.0f, 0.58f, 0.68f };
    const COLORRGB _device_color = { 0.0f, 0.78f, 0.88f };
    const float _back_z = _z - 1.22f;

    glLineWidth(1.0f);
    draw_line({ -3.72f, 0.42f, _z }, { -3.08f, 0.82f, _z - 0.18f }, _trim_color);
    draw_line({ -3.08f, 0.82f, _z - 0.18f }, { -2.82f, 2.05f, _back_z }, _trim_color);
    draw_line({ -2.82f, 2.05f, _back_z }, { -3.32f, 2.42f, _back_z }, _trim_color);
    draw_line({ -3.32f, 2.42f, _back_z }, { -3.72f, 1.95f, _z - 0.32f }, _trim_color);

    draw_line({ 3.72f, 0.42f, _z }, { 3.08f, 0.82f, _z - 0.18f }, _trim_color);
    draw_line({ 3.08f, 0.82f, _z - 0.18f }, { 2.82f, 2.05f, _back_z }, _trim_color);
    draw_line({ 2.82f, 2.05f, _back_z }, { 3.32f, 2.42f, _back_z }, _trim_color);
    draw_line({ 3.32f, 2.42f, _back_z }, { 3.72f, 1.95f, _z - 0.32f }, _trim_color);

    draw_line({ -3.66f, 0.94f, _z - 0.22f }, { -3.66f, 1.72f, _back_z }, _panel_color);
    draw_line({ -3.66f, 1.72f, _back_z }, { -3.18f, 1.82f, _back_z }, _panel_color);
    draw_line({ -3.18f, 1.82f, _back_z }, { -3.18f, 0.86f, _z - 0.34f }, _panel_color);
    draw_line({ -3.18f, 0.86f, _z - 0.34f }, { -3.66f, 0.94f, _z - 0.22f }, _panel_color);

    draw_line({ 3.66f, 0.94f, _z - 0.22f }, { 3.66f, 1.72f, _back_z }, _panel_color);
    draw_line({ 3.66f, 1.72f, _back_z }, { 3.18f, 1.82f, _back_z }, _panel_color);
    draw_line({ 3.18f, 1.82f, _back_z }, { 3.18f, 0.86f, _z - 0.34f }, _panel_color);
    draw_line({ 3.18f, 0.86f, _z - 0.34f }, { 3.66f, 0.94f, _z - 0.22f }, _panel_color);

    glLineWidth(1.4f);
    draw_line({ -3.36f, 1.18f, _z - 0.48f }, { -3.36f, 1.46f, _z - 0.54f }, _device_color);
    draw_line({ -3.48f, 1.34f, _z - 0.50f }, { -3.24f, 1.34f, _z - 0.54f }, _device_color);
    draw_line({ 3.36f, 1.18f, _z - 0.48f }, { 3.36f, 1.46f, _z - 0.54f }, _device_color);
    draw_line({ 3.48f, 1.34f, _z - 0.50f }, { 3.24f, 1.34f, _z - 0.54f }, _device_color);
}

void OPENGLRENDERCOMPONENT::draw_earth_preview() const
{
    const COLORRGB _earth_color = { 0.08f, 0.34f, 0.95f };
    const COLORRGB _land_color = { 0.0f, 0.72f, 0.45f };
    const float _z = -6.32f;
    const float _center_x = 0.0f;
    const float _center_y = 1.72f;
    const float _radius = 0.42f;

    glLineWidth(1.4f);
    set_color(_earth_color);
    glBegin(GL_LINE_LOOP);
    for (int _index = 0; _index < 32; ++_index)
    {
        const float _angle = cj_math_helper::degree_to_radian(static_cast<float>(_index) * 11.25f);
        glVertex3f(_center_x + std::cos(_angle) * _radius, _center_y + std::sin(_angle) * _radius, _z);
    }
    glEnd();

    glLineWidth(1.0f);
    glBegin(GL_LINE_STRIP);
    set_color(_land_color);
    glVertex3f(-0.20f, 1.86f, _z);
    glVertex3f(-0.05f, 1.95f, _z);
    glVertex3f(0.14f, 1.82f, _z);
    glVertex3f(0.05f, 1.63f, _z);
    glVertex3f(0.24f, 1.52f, _z);
    glEnd();
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

void OPENGLRENDERCOMPONENT::draw_shader_glass_overlay() const
{
    if (glass_shader_program == 0 || _gl_use_program == nullptr)
    {
        return;
    }

    _gl_use_program(glass_shader_program);

    const GLint _viewport_location = _gl_get_uniform_location(glass_shader_program, "u_viewport");
    const GLint _time_location = _gl_get_uniform_location(glass_shader_program, "u_time");
    if (_viewport_location >= 0)
    {
        _gl_uniform_2f(_viewport_location, static_cast<float>(viewport_width), static_cast<float>(viewport_height));
    }

    if (_time_location >= 0)
    {
        _gl_uniform_1f(_time_location, static_cast<float>(GetTickCount64()) * 0.001f);
    }

    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(static_cast<float>(viewport_width), 0.0f);
    glVertex2f(static_cast<float>(viewport_width), static_cast<float>(viewport_height));
    glVertex2f(0.0f, static_cast<float>(viewport_height));
    glEnd();

    _gl_use_program(0);
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
    const float _width = static_cast<float>(viewport_width);
    const float _height = static_cast<float>(viewport_height);

    draw_shader_glass_overlay();

    glBegin(GL_QUADS);
    glColor4f(0.02f, 0.18f, 0.22f, 0.08f);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(_width, 0.0f);
    glVertex2f(_width, _height);
    glVertex2f(0.0f, _height);

    glColor4f(0.0f, 0.02f, 0.03f, 0.34f);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(_left * 0.42f, 0.0f);
    glVertex2f(_left * 0.62f, _height);
    glVertex2f(0.0f, _height);

    glVertex2f(_right * 1.07f, 0.0f);
    glVertex2f(_width, 0.0f);
    glVertex2f(_width, _height);
    glVertex2f(_right * 1.02f, _height);

    glColor4f(0.0f, 0.12f, 0.16f, 0.22f);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(_left, 0.0f);
    glVertex2f(_left * 0.72f, _height);
    glVertex2f(0.0f, _height);

    glVertex2f(_right, 0.0f);
    glVertex2f(_width, 0.0f);
    glVertex2f(_width, _height);
    glVertex2f(_right * 1.03f, _height);

    glColor4f(0.0f, 0.08f, 0.10f, 0.18f);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(_width, 0.0f);
    glVertex2f(_width, _top * 0.55f);
    glVertex2f(0.0f, _top * 0.75f);

    glVertex2f(0.0f, _bottom);
    glVertex2f(_width, _bottom * 0.98f);
    glVertex2f(_width, _height);
    glVertex2f(0.0f, _height);

    glColor4f(0.62f, 0.96f, 1.0f, 0.08f);
    glVertex2f(_left * 1.24f, _top * 1.12f);
    glVertex2f(_center_x * 0.86f, _top * 0.76f);
    glVertex2f(_center_x * 0.72f, _center_y * 0.82f);
    glVertex2f(_left * 1.02f, _center_y * 0.96f);

    glColor4f(0.78f, 0.98f, 1.0f, 0.055f);
    glVertex2f(_center_x * 1.18f, _top * 0.82f);
    glVertex2f(_right * 0.94f, _top * 1.30f);
    glVertex2f(_right * 0.82f, _center_y * 0.92f);
    glVertex2f(_center_x * 1.05f, _center_y * 0.74f);
    glEnd();

    glLineWidth(2.2f);
    glColor4f(0.84f, 1.0f, 1.0f, 0.18f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(_left * 0.88f, _center_y * 0.70f);
    glVertex2f(_left * 1.30f, _top * 1.18f);
    glVertex2f(_center_x * 0.84f, _top * 0.74f);
    glVertex2f(_center_x * 1.16f, _top * 0.74f);
    glVertex2f(_right * 0.90f, _top * 1.18f);
    glVertex2f(_right * 1.03f, _center_y * 0.70f);
    glVertex2f(_right * 1.00f, _bottom * 0.76f);
    glVertex2f(_right * 0.92f, _bottom * 0.94f);
    glVertex2f(_center_x, _bottom * 0.99f);
    glVertex2f(_left * 1.10f, _bottom * 0.94f);
    glVertex2f(_left * 0.86f, _bottom * 0.76f);
    glEnd();

    glLineWidth(1.0f);
    glColor4f(0.76f, 0.96f, 1.0f, 0.22f);
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
    glColor4f(0.95f, 1.0f, 1.0f, 0.36f);
    glBegin(GL_LINES);
    glVertex2f(_left * 1.38f, _top * 1.15f);
    glVertex2f(_left * 2.45f, _top * 0.82f);
    glVertex2f(_left * 1.22f, _top * 1.55f);
    glVertex2f(_left * 1.86f, _top * 1.36f);
    glColor4f(0.95f, 1.0f, 1.0f, 0.18f);
    glVertex2f(_center_x * 1.12f, _top * 1.06f);
    glVertex2f(_right * 0.94f, _top * 1.56f);
    glVertex2f(_center_x * 1.02f, _top * 1.38f);
    glVertex2f(_right * 0.84f, _top * 1.88f);
    glEnd();

    glLineWidth(0.75f);
    glColor4f(0.68f, 0.94f, 1.0f, 0.12f);
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

    glLineWidth(0.5f);
    glColor4f(0.80f, 0.98f, 1.0f, 0.06f);
    glBegin(GL_LINES);
    for (int _index = 0; _index < 5; ++_index)
    {
        const float _line_y = _center_y * 0.58f + static_cast<float>(_index) * (_height * 0.075f);
        glVertex2f(_left * 1.18f, _line_y);
        glVertex2f(_right * 0.92f, _line_y + _height * 0.02f);
    }
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
            { L"Warp Engine Control", { -0.95f, 0.72f, -1.92f }, true },
            { L"Auto Turret Target Array", { 0.95f, 0.72f, -1.92f }, true },
            { L"Long Range Radar Lookup", { 0.0f, 0.62f, -1.58f }, true },
            { L"Life Support Cache", { -0.62f, 0.52f, -1.35f }, false },
            { L"Shield Event Queue", { 0.62f, 0.52f, -1.35f }, false },
    };

    return _nodes;
}
