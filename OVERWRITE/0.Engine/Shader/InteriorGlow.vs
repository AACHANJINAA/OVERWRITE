#version 120

uniform float u_time;
varying vec4 v_color;
varying float v_depth;
varying float v_panel;

void main()
{
    vec4 view_position = gl_ModelViewMatrix * gl_Vertex;
    v_color = gl_Color;
    v_depth = length(view_position.xyz);
    v_panel = sin((gl_Vertex.z * 3.2) + (gl_Vertex.y * 5.4) + (u_time * 1.4)) * 0.5 + 0.5;
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
