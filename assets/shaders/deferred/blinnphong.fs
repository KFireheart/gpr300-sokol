#version 410

const int MAX_LIGHTS = 300;

struct Light {
    vec3 position;
    vec3 color;
};

in vec2 TexCoord;
out vec4 FragLighting;

uniform sampler2D g_position;
uniform sampler2D g_normal;
uniform sampler2D g_albedo;
uniform sampler2D g_material;

uniform Light lights[MAX_LIGHTS];
uniform int num_lights;
uniform vec3 camera_position;

vec3 blinnphong(Light light, vec3 P, vec3 N, vec3 V, vec3 albedo, vec4 mat)
{
    vec3 L = normalize(light.position - P);
    vec3 H = normalize(L + V);

    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(N, H), 0.0), max(mat.w * 128.0, 1.0));

    float dist  = length(light.position - P);
    float atten = 1.0 / (1.0 + 0.09 * dist + 0.032 * dist * dist);

    vec3 diffuse  = mat.y * diff * albedo * light.color;
    vec3 specular = mat.z * spec * light.color;

    return (diffuse + specular) * atten;
}

void main()
{
    vec3 P      = texture(g_position, TexCoord).rgb;
    vec3 N      = normalize(texture(g_normal, TexCoord).rgb);
    vec3 albedo = texture(g_albedo,   TexCoord).rgb;
    vec4 mat    = texture(g_material, TexCoord);

    vec3 V = normalize(camera_position - P);

    vec3 result = mat.x * albedo;

    for (int i = 0; i < num_lights; ++i) {
        result += blinnphong(lights[i], P, N, V, albedo, mat);
    }

    FragLighting = vec4(result, 1.0);
}