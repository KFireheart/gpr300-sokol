#version 410

precision mediump float;

out vec4 FragColor;

in vec2 vs_texcoord;

uniform sampler2D screen;
//uniform float offset;
const float offset = 1.0 / 300.0;

uniform float strength;

const vec2 offsets[9] = vec2[] 
(
    vec2(-offset,offset), //top-left
    vec2(0, offset), //top-middle
    vec2(offset,offset), //top-right

    vec2(-offset,0), //center-left
    vec2(0,0), //center-middle
    vec2(offset,0), //center-right

    vec2(-offset, -offset), //bottom-left
    vec2(0, -offset), //bottom-middle
    vec2(offset, -offset) //bottom-right
);


void main()
{
  vec2 dir = vs_texcoord - vec2(0.5);
    float dist = length(dir);
    vec2 offset = normalize(dir) * dist * strength;

    float r = texture(screen, vs_texcoord - offset).r;
    float g = texture(screen, vs_texcoord).g;
    float b = texture(screen, vs_texcoord + offset).b;

    FragColor = vec4(r, g, b, 1.0);
}