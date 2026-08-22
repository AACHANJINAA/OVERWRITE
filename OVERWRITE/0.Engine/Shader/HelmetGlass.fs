#version 120

uniform vec2 u_viewport;
uniform float u_time;
varying vec2 v_uv;

void main()
{
    vec2 aspect = vec2(u_viewport.x / u_viewport.y, 1.0);
    vec2 centered = (v_uv - vec2(0.5, 0.52)) * aspect;
    float radius = length(centered);
    float edge = smoothstep(0.42, 0.82, radius);
    float rim = smoothstep(0.50, 0.86, radius) * 0.22;
    float top_shadow = smoothstep(0.24, 0.0, v_uv.y) * 0.16;
    float bottom_shadow = smoothstep(0.72, 1.0, v_uv.y) * 0.18;
    float left_shadow = smoothstep(0.18, 0.0, v_uv.x) * 0.18;
    float right_shadow = smoothstep(0.82, 1.0, v_uv.x) * 0.18;
    float diag_a = smoothstep(0.018, 0.0, abs((v_uv.y - 0.22) - (v_uv.x - 0.14) * 0.24));
    float diag_b = smoothstep(0.014, 0.0, abs((v_uv.y - 0.30) - (v_uv.x - 0.10) * 0.21));
    float scan = sin((v_uv.y + u_time * 0.04) * 160.0) * 0.5 + 0.5;
    vec3 glass_color = vec3(0.08, 0.34, 0.40);
    vec3 shine_color = vec3(0.82, 1.0, 1.0);
    float alpha = 0.055 + edge * 0.14 + rim + top_shadow + bottom_shadow + left_shadow + right_shadow;
    float shine = diag_a * 0.18 + diag_b * 0.10 + scan * 0.018;

    gl_FragColor = vec4(mix(glass_color, shine_color, shine), clamp(alpha + shine, 0.0, 0.58));
}
