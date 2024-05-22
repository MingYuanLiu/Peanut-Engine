#version 450
#extension GL_GOOGLE_include_directive : enable

#include "structs.h"

layout(binding = 0) uniform _TransformUBO { TransformUBO transform_ubo; };

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_tangent;
layout(location = 3) in vec2 in_texcoord;

layout(location = 0) out vec3 out_position;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec3 out_tangent;
layout(location = 3) out vec2 out_texcoord;

void main()
{
    out_position = (transform_ubo.model * vec4(in_position, 1.0)).xyz;
    out_texcoord = in_texcoord;

    out_normal = normalize(mat3(transform_ubo.normal_model) * in_normal);
    out_tangent = normalize(mat3(transform_ubo.normal_model) * in_tangent);

    gl_Position = transform_ubo.model_view_projection * vec4(in_position, 1.0);
}