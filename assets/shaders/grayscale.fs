#version 410 core

out vec4 FragColor;


// varyings
in vec2 vs_texcoord;

uniform sampler2D the_screen;

void main()
{
  vec3 color = texture(the_screen, vs_texcoord).rgb;
  vec3 grayscale = vec3(color.r*0.2126 + color.g*0.7152 + color.b*0.0722);
  FragColor = vec4(grayscale, 1.0);
}