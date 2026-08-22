#version 120

uniform vec2 u_viewport;
varying vec2 v_uv;

void main()
{
    v_uv = gl_Vertex.xy / u_viewport;
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
