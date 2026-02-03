#version 410 core

out vec4 FragColor;

struct Light{
  vec3 color;
  vec3 position;
};

struct Material{
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
};

struct Palette{
  vec3 color1;
  vec3 color2;
};

// varyings
in vec3 vs_position;
in vec3 vs_normal;
in vec2 vs_texcoord;

uniform vec3 camera;
uniform Light light;
uniform Material material;

uniform Palette pal;

uniform sampler2D gradient;
uniform sampler2D zatoon;

vec3 toonShading(vec3 normal, vec3 frag_position, Light light) {

  vec3 view_dir = normalize(camera - frag_position);
  vec3 light_dir = normalize(light.position - frag_position);
  vec3 reflect_dir = reflect(light_dir, normal);
  vec3 half_dir = normalize(light_dir + view_dir);

  float NdotL = (dot(normal, light_dir) * 1.0) *  0.5;
  float NdotH = (dot(normal, half_dir) + 1.0) * 0.5;


  vec3 gradient = texture(zatoon, vec2(NdotL, NdotL)).rgb;
  vec3 out_color = mix(pal.color2, pal.color1, gradient);


  // vec3 diffuse = NdotL * material.diffuse;
  // float specular_factor = pow(NdotH, material.shininess);
  // vec3 specular = specular_factor * material.specular;
  // vec3 lighting = diffuse + specular;

  return out_color;
}

void main()
{
  vec3 lighting = toonShading(vs_normal, vs_position, light);
  vec3 final_color = lighting + material.ambient;
  FragColor = vec4(final_color, 1.0);
}