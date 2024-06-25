#version 450 core

layout(location=0) in vec3 localPosition;
layout(location=0) out vec4 color;

layout(binding = 0) uniform samplerCube environment_map

void main()
{
    vec3 env_vector = normalize(localPosition);
    color = textureLod(environment_map, env_vector);
}
