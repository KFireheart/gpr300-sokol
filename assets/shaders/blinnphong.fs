#version 410

precision mediump float;

out vec4 FragColor;

struct Light{
  vec3 color;
  vec3 position;
};

struct Material{
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininness;
};

// varyings
in vec3 vs_position;
in vec3 vs_normal;
in vec2 vs_texcoord;

uniform vec3 camera;
uniform Light light;
uniform Material material;

vec3 blinnphong(vec3 normal, vec3 frag_position, Light light) {

  vec3 view_dir = normalize(camera - frag_position);
  vec3 light_dir = normalize(light.position - frag_position);
  vec3 reflect_dir = reflect(light_dir, normal);
  vec3 half_dir = normalize(light_dir + view_dir);

  float NdotL = max(dot(normal, light_dir), 0.0);
  float NdotH = max(dot(normal, half_dir), 0.0);

  vec3 diffuse = NdotL * material.diffuse;
  vec3 specular = pow(NdotH, material.shininness) * material.alpha;
  vec3 lighting = vec3(diffuse) + vec3(specular);

  return lighting * light.color;
}

void main()
{
  vec3 lighting = blinnphong(vs_normal, vs_position, light) + ambient;
  vec3 object_color = vs_normal.rgb * lighting;
  vec3 final_color = object_color * lighting;
  FragColor = vec4(final_color, 1.0);
}