#version 120

uniform vec2 u_center;
uniform float u_radius;
uniform float u_time;
varying vec2 v_world;

float hash(vec2 value)
{
    return fract(sin(dot(value, vec2(127.1, 311.7))) * 43758.5453);
}

float noise(vec2 value)
{
    vec2 cell = floor(value);
    vec2 fraction = fract(value);
    vec2 curve = fraction * fraction * (3.0 - 2.0 * fraction);
    float a = hash(cell);
    float b = hash(cell + vec2(1.0, 0.0));
    float c = hash(cell + vec2(0.0, 1.0));
    float d = hash(cell + vec2(1.0, 1.0));
    return mix(mix(a, b, curve.x), mix(c, d, curve.x), curve.y);
}

void main()
{
    vec2 uv = (v_world - u_center) / u_radius;
    float dist = length(uv);

    if (dist > 1.18)
    {
        discard;
    }

    float sphere = sqrt(max(0.0, 1.0 - dist * dist));
    vec3 normal = normalize(vec3(uv.x, uv.y, sphere));
    vec3 light_dir = normalize(vec3(-0.55, 0.38, 0.74));
    float light = clamp(dot(normal, light_dir), 0.0, 1.0);
    float limb = 1.0 - smoothstep(0.62, 1.0, dist);
    float atmosphere = smoothstep(0.82, 1.10, dist) * (1.0 - smoothstep(1.10, 1.18, dist));

    vec2 map_uv = uv * vec2(1.4, 1.0) + vec2(u_time * 0.012, 0.0);
    float continent_a = noise(map_uv * 3.1);
    float continent_b = noise(map_uv * 7.3 + vec2(4.2, 1.6));
    float continent = smoothstep(0.56, 0.74, continent_a * 0.68 + continent_b * 0.42);
    float cloud = smoothstep(0.54, 0.74, noise(map_uv * 10.5 + vec2(u_time * 0.035, 6.0)));

    vec3 ocean_color = mix(vec3(0.015, 0.13, 0.48), vec3(0.05, 0.42, 0.92), light);
    vec3 land_color = mix(vec3(0.02, 0.23, 0.10), vec3(0.34, 0.78, 0.34), light);
    vec3 cloud_color = vec3(0.82, 0.95, 1.0) * (0.42 + light * 0.78);
    vec3 color = mix(ocean_color, land_color, continent);
    color = mix(color, cloud_color, cloud * 0.42);
    color *= 0.18 + light * 1.25;
    color += vec3(0.12, 0.54, 1.0) * atmosphere * 0.95;
    color += vec3(0.02, 0.12, 0.38) * (1.0 - limb) * 0.55;

    float alpha = 1.0 - smoothstep(1.0, 1.18, dist);
    gl_FragColor = vec4(color, alpha);
}
