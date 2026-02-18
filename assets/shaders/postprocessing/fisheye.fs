#version 410

precision mediump float;

out vec4 FragColor;

in vec2 vs_texcoord;

uniform sampler2D screen;
uniform float strength; 


void main()
{
    vec2 uv = vs_texcoord - vec2(0.5);
    float dist = length(uv);

    
    uv = uv * (1.0 + strength * dist * dist);

    uv += vec2(0.5);

    
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
    {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0); 
        return;
    }

    FragColor = vec4(texture(screen, uv).rgb, 1.0);
}