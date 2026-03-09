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
in vec4 light_projection_pos;


uniform vec3 camera;
uniform Light light;
uniform Material material;

uniform Palette pal;

uniform sampler2D gradient;
uniform sampler2D zatoon;

uniform sampler2D shadow_map;

uniform float bias;

float shadowCalculation(vec4 fragPosLightSpace){
  // perspective divide
  vec3 proj_coords = fragPosLightSpace.xyz / fragPosLightSpace.w;

  // transform from NDC (-1..1) to texture space (0..1)
  proj_coords = proj_coords * 0.5 + 0.5;

  // if fragment is outside the light frustum, treat as unshadowed
  if (proj_coords.z > 1.0 ||
      proj_coords.x < 0.0 || proj_coords.x > 1.0 ||
      proj_coords.y < 0.0 || proj_coords.y > 1.0) {
    return 0.0;
  }

  float closest_depth = texture(shadow_map, proj_coords.xy).r;
  float currentDepth = proj_coords.z;

  // simple bias to reduce shadow acne
  float shadow = (currentDepth - bias > closest_depth) ? 1.0 : 0.0;

  return shadow;
}

vec3 toonShading(vec3 normal, vec3 frag_position, vec3 light_position) {

  vec3 view_dir = normalize(camera - frag_position);
  vec3 light_dir = normalize(light_position - frag_position);
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

  vec3 normal = normalize(vs_normal);

  //vec3 lighting = toonShading(vs_normal, vs_position, light);
  float shadow = shadowCalculation(light_projection_pos);
  vec3 light_color = toonShading(normal, vs_position, light.position);

  light_color *= (1.0 - shadow);
  
  FragColor = vec4(light_color, 1.0);
}