#version 410

precision mediump float;

out vec4 FragColor;

in vec2 vs_texcoord;

uniform sampler2D screen;
uniform float time;       
uniform float grain_strength; 

float random(vec2 uv, float seed)
{
    return fract(sin(dot(uv + seed, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    vec3 color = texture(screen, vs_texcoord).rgb;

    float grain = random(vs_texcoord, time);

    // Remap 0–1 noise to -0.5–0.5 so it brightens AND darkens
    grain = (grain - 0.5) * grain_strength;

    color += grain;

    FragColor = vec4(color, 1.0);
}