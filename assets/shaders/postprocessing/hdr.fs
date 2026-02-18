#version 410

precision mediump float;

out vec4 FragColor;

in vec2 vs_texcoord;

uniform sampler2D screen;
//uniform float offset;

uniform float exposure;
uniform float gamma;

void main()
{
    vec3 hdrColor = texture(screen, vs_texcoord).rgb ;
  
    // reinhard tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
  
    FragColor = vec4(mapped, 1.0);
}