#version 120

varying vec2 v_world;

void main()
{
    v_world = gl_Vertex.xy;
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
