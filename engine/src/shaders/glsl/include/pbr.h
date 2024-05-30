#ifndef _PBR_
#define _PBR_

#include "host_device_structs.h"

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

vec3 CalIBLContribution()
{
    return 
}

// Basic Lambertian diffuse color
vec3 CalDiffuseColor(vec3 diffuse_color)
{
    return diffuse_color / PI;
}

// GGX（Trowbridge-Reitz) normal distribution
float CalMicrofacetNormalDistribution(PbrInfo pbr_info)
{
    float sqaure_roughness = pbr_info.alpha_roughness * pbr_info.alpha_roughness;
    float denominator = (pbr_info.NdotH * sqaure_roughness - pbr_info.NdotH) * pbr_info.NdotH + 1.0;

    return sqaure_roughness / (PI * denominator * denominator);
}

// reflection color of fresh
// F = F0 + (1 - F0) * (1 - cos(thea))^5
// rapid version from epic game:
// Karis B, Games E. Real shading in unreal engine 4[J]. Proc. Physically Based Shading Theory Practice, 2013, 4. https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
float CalFreshReflection(PbrInfo pbr_info)
{
    vec3 F0 = pbr_info.reflection;
    vec3 F90 = pbr_info.reflectance_90_angle;
    return F0 + (F90 - F0) * exp2(-5.55473 * pbr_info.VdotH - 6.98316 * pbr_info.VdotH );
}

// Specular color of pbr
// color = D*F*G / (4*NdotV*NdotL)
// @arg [n]: normal
// @arg [v]: view direction
// @arg [l]: light direction
// @arg [radiance]: intensity of incident radiance
vec3 CalSpecularColor(PbrInfo pbr_info, vec3 n, vec3 v, vec3 l, vec3 radiance)
{
    // half vector
    vec3 h = normalize(v + l);
    pbr_info.NdotL = clamp(dot(n, l), 0,001, 1.0);
    pbr_info.NdotV = clamp(dot(n, v), 0,0, 1.0);
    pbr_info.NdotH = clamp(dot(n, h), 0,0, 1.0);
    pbr_info.VdotL = clamp(dot(v, l), 0,0, 1.0);

    // calculate Normal Distribution Function
    float D = CalMicrofacetNormalDistribution(pbr_info);
}


#endif