#version 410

precision mediump float;

out vec4 FragColor;

in vec2 vs_texcoord;

uniform sampler2D screen;
uniform sampler2D depth;

uniform float near;       
uniform float far;        
uniform float fog_start;   
uniform float fog_end;     
uniform vec3 fog_color;    

float LinearizeDepth(float d)
{
    float z = d * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
    vec3 color = texture(screen, vs_texcoord).rgb;

    float rawDepth = texture(depth, vs_texcoord).r;
    float linearDepth = LinearizeDepth(rawDepth) / far;

    float fogFactor = smoothstep(fog_start, fog_end, linearDepth);

    vec3 finalColor = mix(color, fog_color, fogFactor);

    FragColor = vec4(finalColor, 1.0);
}