#version 450
#extension GL_GOOGLE_include_directive : enable

#include "structs.h"

layout(binding = 0) uniform _MaterialPCO { MaterialPCO material_pco; };
layout(binding = 1) uniform sampler2D base_color_texture_sampler;
layout(binding = 2) uniform sampler2D normal_texture_sampler;
layout(binding = 3) uniform sampler2D metallic_roughness_occlusion_texture_sampler;
layout(binding = 5) uniform sampler2D emissive_color_texture_sampler;

// input data from vertex shader
layout(location = 0) in highp vec3 in_position;
layout(location = 1) in highp vec3 in_normal;
layout(location = 2) in highp vec3 in_tangent;
layout(location = 3) in highp vec2 in_texcoord;

// gbuffer
layout(location = 0) out highp vec4 out_gbuffer_a; // normal
layout(location = 1) out highp vec4 out_gbuffer_b; // metallic, roughness, occlusion
layout(location = 2) out highp vec4 out_gbuffer_c; // base color
layout(location = 3) out highp vec4 out_emissive_color;

highp vec3 calculate_normal()
{
    highp vec3 tagent_normal = texture(normal_texture_sampler, in_texcoord).xyz * 2 - 1.0;

    highp vec3 N = normalize(in_normal);
    highp vec3 T = normalize(in_tangent.xyz/* maybe not need 'xyz' */);
    highp vec3 B = normalize(cross(N, T));

    highp mat3 TBN = mat3(T, B, N);
    return normalize(TBN * tagent_normal);
}

void main()
{
    highp vec3 normal = calculate_normal();
    
    // normal
    out_gbuffer_a = vec4(normal, 0.0);
    
    // base color
    out_gbuffer_c = material_pco.base_color_factor;
    if (bool(material_pco.has_base_color_texture))
    {
        out_gbuffer_c *= texture(base_color_texture_sampler, in_texcoord);
    }

    // emissive color
    out_emissive_color = material_pco.emissive_factor;
    if (bool(material_pco.has_emissive_texture))
    {
        out_emissive_color = texture(emissive_color_texture_sampler, in_texcoord);
    }
    
    // metallic roughness and occlusion
    vec3 metallic_roughness_occlusion = vec3(material_pco.metallic_factor, material_pco.roughness_factor, 1.0);
    if (bool(material_pco.has_metallic_roughness_occlusion_texture))
    {
        vec4 mro_data = texture(metallic_roughness_occlusion_texture_sampler, in_texcoord);
        metallic_roughness_occlusion.xyz *= vec3(mro_data.b, mro_data.g, bool(material_pco.contains_occlusion_channel) ? mro_data.r : 1.0);
    }

    out_gbuffer_b.xyz = metallic_roughness_occlusion;
}
