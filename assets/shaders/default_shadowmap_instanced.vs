#version 410 core

// attributes
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;
layout(location = 3) in mat4 in_instanceMatrix;

// uniforms
uniform mat4 view_proj;
uniform mat4 model;
uniform mat4 light_view_proj;

// varyings
out vec3 vs_position;
out vec3 vs_normal;
out vec2 vs_texcoord;
out vec4 light_projection_pos;

void main()
{
    // world-space position and normal
    vec4 world_pos = in_instanceMatrix * vec4(in_position, 1.0);
    vs_position = world_pos.xyz;
    vs_normal = transpose(inverse(mat3(in_instanceMatrix))) * in_normal;
    vs_texcoord = in_texcoord;

    // light-space position for shadow lookup
    light_projection_pos = light_view_proj * world_pos;

    // regular camera transform
    gl_Position = view_proj * world_pos;

}