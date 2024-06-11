#ifndef _PBR_
#define _PBR_

#include "host_device_structs.h"
#include "hdr.h"

// global resource - ibl texture
layout(set = 0, binding = 5) uniform samplerCube irradiance_texture_sampler;
layout(set = 0, binding = 6) uniform samplerCube perfilter_texture_sampler;
layout(set = 0, binding = 7) uniform sampler2D brdf_LUT_texture_sampler;

// shadow texture
layout(set = 0, binding = 8) uniform sampler2DArray directonal_light_shadow_texture_sampler;
// todo: point light
// todo: spot light

// light ubo
layout(set = 0, binding = 9) uniform _LightingUBO {LightingUBO lighting_ubo;};

struct PbrInfo
{
    float NdotL; // cos(angle) between normal and light direction
    float NdotV; // cos(angle) between normal and view direction
    float NdotH; // cos(angle) between normal and half vector
    float VdotL; // cos(angle) between view direction and light direction
    float VdotH; // cos(angle) between view direction and half vector

    float roughness; // roughness value from metallic&roughness texture
    float alpha_roughness; // square of roughness which mapped to a more linear change in the roughness
    float metallic; // metallic value from metallic&roughness texture

    vec3 reflection;              //  Fresnel reflectance color (normal incidence angle)
	vec3 reflectance_90_angle;   // reflectance color at grazing angle
	vec3 diffuse_color;           // color contribution from diffuse lighting
	vec3 specular_color;          // color contribution from specular lighting
}

// Calculation of the lighting contribution from an optional Image Based Light source.
// Precomputed Environment Maps are required uniform inputs and are computed as outlined in [1].
vec3 CalIBLContribution(PbrInfo pbr_info, vec3 n, vec3 r)
{
    float lod = pbr_info.roughness * lighting_ubo.sky_light.prefilter_mip_levels;

    vec3 brdf = (texture(brdf_LUT_texture_sampler, vec2(pbr_info.NdotV, 1.0 - pbr_info.roughness))).rgb;
    vec3 diffuse_light = SRGBToLinear(Tonemap(texture(irradiance_texture_sampler, n).rgb));
    vec3 specular_light = SRGBToLinear(Tonemap(textureLod(perfilter_texture_sampler, r, lod).rgb));

    vec3 diffuse_color = diffuse_light * pbr_info.diffuse_color;
    vec3 specular_color = specular_light * (pbr_info.specular_color * brdf.x + brdf.y);

    return (diffuse_color + specular_color) * lighting_ubo.sky_light.color;
}

// Basic Lambertian diffuse color
vec3 CalDiffuseColor(vec3 diffuse_color)
{
    return diffuse_color / PI;
}

// GGX（Trowbridge-Reitz) normal distribution
float CalMicrofacetNormalDistribution(float NdotH, float alpha_roughness)
{
    float sqaure_roughness = alpha_roughness * alpha_roughness;
    float denominator = (NdotH * sqaure_roughness - NdotH) * NdotH + 1.0;

    return sqaure_roughness / (PI * denominator * denominator);
}

// reflection color of Fresnel
// F = F0 + (1 - F0) * (1 - cos(thea))^5
// rapid version from epic game:
// Karis B, Games E. Real shading in unreal engine 4[J]. Proc. Physically Based Shading Theory Practice, 2013, 4. https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
float CalFresnelReflection(vec3 F0, vec3 F90, float VdotH)
{
    // return F0 + (F90 - F0) * exp2(-5.55473 * pbr_info.VdotH - 6.98316 * pbr_info.VdotH );
    return F0 + (F90 - F0) * pow(clamp(1 - VdotH, 0.0, 1.0), 5);
}

// Calculate geometric attenuation for specular BRDF.
float CalGeometricAttenuation(float NdotL, float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    float GL = NdotL / (NdotL * (1.0 - k) + k);
    float GV = NdotV / (NdotV * (1.0 - k) + k);
    return GL * GV;
}

// get light contribute using cook-torrance
// L = diffuse + specular
//
// Specular color of pbr
// color = D*F*G / (4*NdotV*NdotL)
// @arg [n]: normal
// @arg [v]: view direction
// @arg [l]: light direction
// @arg [radiance]: intensity of incident radiance
vec4 GetLightByCookTorrance(PbrInfo pbr_info, vec3 n, vec3 v, vec3 l, vec3 radiance)
{
    // half vector
    vec3 h = normalize(v + l);

    // calculate cos angle
    pbr_info.NdotL = clamp(dot(n, l), 0,001, 1.0);
    pbr_info.NdotV = clamp(dot(n, v), 0,0, 1.0);
    pbr_info.NdotH = clamp(dot(n, h), 0,0, 1.0);
    pbr_info.VdotL = clamp(dot(v, l), 0,0, 1.0);
    pbr_info.VdotH = clamp(dot(v, h), 0,0, 1.0);

    // calculate Normal Distribution Function
    float D = CalMicrofacetNormalDistribution(pbr_info.NdotH, pbr_info.alpha_roughness);
    float F = CalFresnelReflection(pbr_info.reflection, pbr_info.reflectance_90_angle);
    float G = CalGeometricAttenuation(pbr_info.NdotL, pbr_info.NdotV, pbr_info.roughness);

    // specular
    vec3 specular_contrib = F * G * D / (4.0 * pbr_info.NdotL * pbr_info.NdotV);
    // diffuse color
    vec3 diffuse_contrib = (1 - F) * CalDiffuseColor(pbr_info.diffuse_color);

    return pbr_info.NdotL * radiance * (diffuse_contrib + specular_contrib);
}

bool IsDebugLight() { return lighting_ubo.shader_debug_option == ShaderDebugOption::Debug_Light; }
bool IsDebugUnlight() { return lighting_ubo.shader_debug_option == ShaderDebugOption::Debug_Unlit; }
bool IsDebugWireframe() { return lighting_ubo.shader_debug_option == ShaderDebugOption::Debug_Wireframe; }
bool IsDebugLightingOnly() { return lighting_ubo.shader_debug_option == ShaderDebugOption::Debug_LightOnly; }
bool IsDebugDepth() { return lighting_ubo.shader_debug_option == ShaderDebugOption::Debbug_Depth; }
bool IsDebugNormal() { return lighting_ubo.shader_debug_option == ShaderDebugOption::Debug_Normal; }
bool IsDebugBaseColor() { return lighting_ubo.shader_debug_option == ShaderDebugOption::Debug_BaseColor; }
bool IsDebugEmissiveColor() { return lighting_ubo.shader_debug_option == ShaderDebugOption::Debug_EmissiveColor; }
bool IsDebugMetallic() { return lighting_ubo.shader_debug_option == ShaderDebugOption::Debug_Metallic; }
bool IsDebugRoughness() { return lighting_ubo.shader_debug_option == ShaderDebugOption::Debug_Roughness; }
bool IsDebugOcclusion() { return lighting_ubo.shader_debug_option == ShaderDebugOption::Debug_Occulsion; }
bool IsDebugOpacity() { return lighting_ubo.shader_debug_option == ShaderDebugOption::Debug_Opacity; }

vec4 CalPbrLightColor(PbrMaterialInfo material_info)
{
    // only output depth
    if (IsDebugDepth())
    {
        float depth = dot(material_info.position - lighting_ubo.camera_pos, lighting_ubo.camera_dir);
        depth *= DEBUG_SHADER_DEPTH_MULTIPLIER;
        return vec4(vec3(depth), 1.0);
    }

    if (IsDebugNormal())
    {
        return vec4((material_info.normal + 1.0) / 2.0, 1.0);
    }

    if (IsDebugBaseColor())
    {
        return vec4(material_info.base_color.xyz, 1.0);
    }

    if (IsDebugEmissiveColor())
    {
        return vec4(material_info.emissive_color.xyz, 1.0);
    }

    if (IsDebugMetallic())
    {
        return vec4(vec3(material_info.metallic), 1.0);
    }

    if (IsDebugRoughness())
    {
        return vec4(vec3(material_info.roughness), 1.0);
    }

    if (IsDebugOcclusion())
    {
        return vec4(vec3(material_info.occlusion), 1.0);
    }

    if (IsDebugOpacity())
    {
        return vec4(vec3(material_info.base_color.a), 1.0);
    }

    if (IsDebugLightingOnly())
    {
        material_info.base_color.rgb = vec3(0.75);
    }

    float alpha_roughness = material_info.roughness * material_info.roughness;
    
    // todo: why calculate using this equation
    // diffuse color and speculat color
    vec3 F0 = vec3(DIELECTRIC_F0);
    vec3 diffuse_color = material_info.base_color.rgb * (vec3(1.0) - F0);
    diffuse_color = diffuse_color * (1.0 - material_info.metallic);

    vec3 specular_color = mix(F0, material_info.base_color.rgb, material_info.metallic);

    float reflection = max(max(specular_color.r, specular_color.g), specular_color.b);
    float reflection_90 = clamp(reflection * 25.0, 0, 1.0);
    vec3 specular_color_reflection_90 = reflectance_90 * vec3(1.0);

    // surface, camera, and light directions
    vec3 n = materail_info.normal;
    vec3 v = normalize(lighting_ubo.camera_pos - material_info.position);
    vec3 r = reflect(-v, n); // reflect direction

    float NdotV = clamp(abs(dot(n, v)), 0.001, 1.0);

    PbrInfo pbr_info;
    pbr_info.NdotV = NdotV;
    pbr_info.roughness = material_info.roughness;
    pbr_info.alpha_roughness = alpha_roughness;
    pbr_info.metallic = material_info.metallic;
    pbr_info.reflection = specular_color;
    pbr_info.reflectance_90_angle = specular_color_reflection_90;
    pbr_info.diffuse_color = diffuse_color;
    pbr_info.specular_color = specular_color;

    vec3 light_color = vec3(0.0);

    // direction light
    if (bool(lighting_ubo.has_directional_light))
    {
        DirectionalLight directional_light = lighting_ubo.directional_light;
        // todo: draw shadow
        
        // brdf
        light_color += GetLightByCookTorrance(pbr_info, n, v, -directional_light.direction, directional_light.color);
    }

    // point light

    // spot light

    // ibl indirect light contribution
    if (bool(lighting_ubo.has_sky_light))
    {
        light_color += CalIBLContribution(pbr_info, n, r);
    }

    vec3 result_color = IsDebugUnlight() ? vec3(0.0) : light_color;
    
    result_color = result_color * material_info.occlusion + material_info.emissive_color;

    result_color = Tonemap(result_color);

    result_color = LinearToSRGB(result_color);

    return vec4(result_color, materail_info.base_color.a);
}


#endif