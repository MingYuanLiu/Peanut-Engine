#version 450 core

#extension GL_GOOGLE_include_directive : enable

#include "host_device_structs.h"
#include "pbr.h"

layout(push_constant) uniform _MaterialPCO { MaterialPCO material_pco; };
layout(set = 0, binding = 0) uniform sampler2D base_color_texture_sampler;
layout(set = 0, binding = 1) uniform sampler2D normal_texture_sampler;
layout(set = 0, binding = 2) uniform sampler2D metallic_roughness_occlusion_texture_sampler;
layout(set = 0, binding = 3) uniform sampler2D emissive_color_texture_sampler;

// input data from vertex shader
layout(location = 0) in highp vec3 in_position;
layout(location = 1) in highp vec3 in_normal;
layout(location = 2) in highp vec3 in_tangent;
layout(location = 3) in highp vec2 in_texcoord;

layout(location = 0) out vec4 o_color;

highp vec3 calculate_normal()
{
    highp vec3 tagent_normal = texture(normal_texture_sampler, in_texcoord).xyz * 2 - 1.0; // clap the normal value to [0, 1]

    highp vec3 N = normalize(in_normal);
    highp vec3 T = normalize(in_tangent.xyz/* maybe not need 'xyz' */);
    highp vec3 B = normalize(cross(N, T));

    highp mat3 TBN = mat3(T, B, N);
    return normalize(TBN * tagent_normal);
}

void main()
{
    vec3 normal = calculate_normal();
    // base color
    PbrMaterialInfo material_info;
    material_info.position = in_position;
    material_info.normal = normal;
    material_info.base_color = material_pco.base_color_factor * texture(base_color_texture_sampler, in_texcoord);

    vec3 metallic_roughness_occlusion = vec3(material_pco.metallic_factor, material_pco.roughness_factor, 1.0);
    vec4 mro_data = texture(metallic_roughness_occlusion_texture_sampler, in_texcoord);
    metallic_roughness_occlusion.xyz *= vec3(mro_data.r, mro_data.g, bool(material_pco.contains_occlusion_channel) ? mro_data.b : 1.0);

    material_info.metallic = metallic_roughness_occlusion.x;
    material_info.roughness = metallic_roughness_occlusion.y;
    material_info.occlusion = metallic_roughness_occlusion.z;

    out_color = CalPbrLightColor(material_info);
}