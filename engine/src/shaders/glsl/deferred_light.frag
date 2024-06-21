#version 450

#extension GL_GOOGLE_include_directive : enable

#include "pbr.h"

// gbuffer
layout(input_attachment_index = 0, binding = 0) uniform subpassInput normal_texture_sampler;
layout(input_attachment_index = 2, binding = 1) uniform subpassInput metallic_roughness_occlusion_texture_sampler;
layout(input_attachment_index = 1, binding = 2) uniform subpassInput base_color_texture_sampler;
layout(input_attachment_index = 2, binding = 3) uniform subpassInput depth_stencil_texture_sampler;

layout(location = 0) in vec2 in_texcoord;
layout(location = 0) out vec4 out_color;

void main()
{
    float depth = subpassLoad(depth_stencil_texture_sampler).x;
    vec4 ndc_position = vec4(in_texcoord * 2.0 - 1.0, depth, 1.0);
    vec4 world_position = lighting_ubo.inv_camera_view_prog * ndc_position;

    PbrMaterialInfo material_info;
    material_info.position = world_position.xyz / world_position.w;
    material_info.normal = subpassLoad(normal_texture_sampler).xyz;
    material_info.base_color = subpassLoad(base_color_texture_sampler);

    vec3 metallic_roughness_occlusion = subpassLoad(metallic_roughness_occlusion_texture_sampler);
    material_info.metallic = metallic_roughness_occlusion.x;
    material_info.roughness = metallic_roughness_occlusion.y;
    material_info.occlusion = metallic_roughness_occlusion.z;

    out_color = CalPbrLightColor(material_info);
}
