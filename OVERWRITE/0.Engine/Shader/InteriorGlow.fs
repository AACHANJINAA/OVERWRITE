#version 120

uniform float u_time;
varying vec4 v_color;
varying float v_depth;
varying float v_panel;

void main()
{
    vec3 base_color = v_color.rgb;
    float depth_fade = clamp(1.0 - (v_depth / 18.0), 0.28, 1.0);
    float pulse = 0.82 + (v_panel * 0.18);
    float electric = sin((v_depth * 2.1) - (u_time * 2.0)) * 0.5 + 0.5;
    vec3 cyan_glow = vec3(0.0, 0.72, 0.90) * electric * 0.14;
    float magenta_push = smoothstep(0.45, 1.0, base_color.r - base_color.g);
    float violet_push = smoothstep(0.30, 1.0, base_color.b - base_color.g);
    vec3 cyber_color = base_color;
    cyber_color += vec3(1.0, 0.02, 0.58) * magenta_push * 0.38;
    cyber_color += vec3(0.42, 0.08, 1.0) * violet_push * 0.34;
    vec3 final_color = (cyber_color * (1.18 + pulse * 0.42) + cyan_glow) * depth_fade;

    gl_FragColor = vec4(final_color, v_color.a);
}
