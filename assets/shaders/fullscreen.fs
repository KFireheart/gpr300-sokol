#version 410 core

out vec4 FragColor;


// varyings
in vec2 vs_texcoord;

uniform sampler2D the_screen;

void main()
{
  vec3 color = texture(the_screen, vs_texcoord).rgb;
  FragColor = vec4(color, 1.0);
}