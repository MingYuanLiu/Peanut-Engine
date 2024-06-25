#version 450 core

#extension GL_GOOGLE_include_directive : enable

#include "host_device_structs.h"

layout(push_constant) uniform _TransformUBO {TransformUBO transform_ubo;};

layout(location = 0) in vec3 position
layout(location = 0) out vec3 localPosition

void main()
{
    localPosition = position.xyz;
    gl_Position = transform_ubo.mvp * vec4(position, 1.0);
}