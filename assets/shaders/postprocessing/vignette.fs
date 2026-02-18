#version 410

precision mediump float;

out vec4 FragColor;

in vec2 vs_texcoord;

uniform sampler2D screen;

const float offset = 1.0 / 300.0;
uniform float strength;     
uniform float falloff;       
const vec2 offsets[9] = vec2[]
(
    vec2(-offset, offset),
    vec2(0,       offset),
    vec2(offset,  offset),

    vec2(-offset, 0),
    vec2(0,       0),
    vec2(offset,  0),

    vec2(-offset, -offset),
    vec2(0,       -offset),
    vec2(offset,  -offset)
);


const float kernel[9] = float[](
    0, 0, 0,
    0, 1, 0,
    0, 0, 0
);

void main()
{
    vec3 color = vec3(0.0);
    for (int i = 0; i < 9; i++)
    {
        vec3 s = vec3(texture(screen, vs_texcoord + offsets[i]));
        color += s * kernel[i];
    }

    vec2 uv = vs_texcoord - vec2(0.5);
    float dist = length(uv);
    float vignette = smoothstep(falloff, falloff - falloff, dist * strength);

    FragColor = vec4(color * vignette, 1.0);
}