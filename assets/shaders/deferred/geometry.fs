#version 410 

precision mediump float;

struct Material {
  float ambient;
  float diffuse;
  float specular;
  float shininess;
};

// attachments
layout(location = 0) out vec4 frag_position;
layout(location = 1) out vec4 frag_normal;
layout(location = 2) out vec4 frag_albedo;
layout(location = 3) out vec4 frag_material;

// varyings
in vec3 vs_position;
in vec3 vs_normal;
in vec2 vs_texcoord;

uniform Material material;

void main()
{
    frag_position = vec4(vs_position, 1.0);
    frag_normal   = vec4(normalize(vs_normal), 1.0);
    frag_albedo   = vec4(0.8, 0.8, 0.8, 1.0);
    frag_material = vec4(material.ambient, material.diffuse, material.specular, material.shininess);
}