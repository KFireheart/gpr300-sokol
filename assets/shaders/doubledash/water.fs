#version 410

out vec4 FragColor;

// varying;
in vec3 vs_position;
in vec3 vs_normal;
in vec2 vs_texcoord;

// uniforms
uniform vec3 camera;
uniform sampler2D wave_tex;
uniform sampler2D wave_spec;
uniform sampler2D wave_warp;

uniform float time;
uniform vec3 water_color;

float scale = 1.0f;

void main()
{
  //offset texcoords in some direction
  vec2 dir = vec2(1.0, 0.0);
  vec2 uv = vs_texcoord;// + vec2(time * dir);

  vec4 sample1 = texture(wave_tex, uv * 1.0);
  vec4 sample2 = texture(wave_tex, uv * 1.2);

  //warp
  vec2 warp_uv = vs_texcoord * scale;
  vec2 warp_scroll = vec2(0.5, 0.5) * time;
  vec2 warp = texture(wave_warp, warp_uv + warp_scroll).xy;
  warp = (warp * 2.0) - 1.0;


 //albedo
  vec2 albedo_uv = vs_texcoord * scale;
  vec3 albedo = texture(wave_tex, albedo_uv + warp).rgb;

  //specular/shimmer
  vec2 spec_uv = vs_texcoord * scale;
  vec3 spec_smp1 = texture(wave_spec, spec_uv + vec2(0.5, 0.5) * time).rgb;
  vec3 spec_smp2 = texture(wave_spec, spec_uv + vec2(-0.5, -0.5) * time).rgb;
  vec3 spec = (spec_smp1 + spec_smp2) * 0.5;

  //fresnel
  float fresnel = dot(normalize(camera), vec3(0.0, 1.0, 0.0));

  const vec3 kbright = vec3(0.299, 0.587, 0.114);
  float brightness = dot(spec, kbright);

  vec3 final_color = albedo;
  if (brightness <= 0.5 || brightness >= 0.95) {
    final_color = mix(final_color, final_color + spec, fresnel);
  }

  FragColor = vec4(final_color, 1.0);
}