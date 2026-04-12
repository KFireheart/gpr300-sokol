#version 410

struct Light
{
    vec3 position;
    vec3 color;
};

uniform sampler2D g_position;
uniform sampler2D g_normal;
uniform sampler2D g_albedo;
uniform sampler2D g_material;
uniform Light light;
uniform vec3 camera_position;

out vec4 FragLighting;

vec3 blinnphong(vec3 position, vec3 normal, vec3 material)
{
    //calculate blinnphong
    return vec4(0.0, 0.3, 0.7, 1.0);

}

void main()
{
    vec3 lighting = blinnphong(g_position, g_normal, g_material);
    FragLighting = vec4(0.0, 1.0, 0.0, 1.0);
}