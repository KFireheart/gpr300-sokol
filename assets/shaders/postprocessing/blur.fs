#version 410


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

const float kernel[9] = float[](
    1,2,1,
    2,4,2,
    1,2,1
);

// const float kernel[9] = float[](
//     1,2,1,
//     2,4,2,
//     1,2,1
// );


// const float kernel[9] = float[](
//     -1,-1,-1,
//     -1,8,-1,
//     -1,-1,-1
// );

void main()
{
  vec3 color = vec3(0);

    for (int i = 0; i < 9; i++)
    {
        vec3 local = vec3(texture(screen, vs_texcoord + offsets[i]));
        color += local * kernel[i] * strength;
    }

  FragColor = vec4(color, 1.0);
}