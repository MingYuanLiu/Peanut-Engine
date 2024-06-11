#ifndef _STRUCT_H_
#define _STRUCT_H_

#include "constants.h"

#ifdef __cplusplus
#include <glm/glm.hpp>
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat4 = glm::mat4;
using uint = unsigned int;
#endif

struct TransformUBO // uniform buffer
{
    mat4 model;
    mat4 normal_model;
    mat4 model_view_projection;
};

struct MaterialPCO // push constant
{
    vec4 base_color_factor;
    vec4 emissive_factor;
	float metallic_factor;
	float roughness_factor;
    float normal_scale;
    float occlusion_strength;

    int has_base_color_texture; 
    int has_emissive_texture;
    int has_metallic_roughness_occlusion_texture;
    int contains_occlusion_channel;
    int has_normal_texture;
    int is_blend;
    int is_double_sided;
};

struct SkyLight
{
    vec3 color;
    float prefilter_mip_levels;
};

struct DirectionalLight
{
    vec3 direction;
    int cast_shadow;
    vec3 color;
    float padding0;
    mat4 cascade_view_projs[SHADOW_CASCADE_NUM];
    vec4 cascade_splits;
};

struct PointLight
{
    vec3 position;
    float padding0;
    vec3 color;
    float padding1;

    float radius;
    float linear_attenuation;
    float quadratice_attenuation;
    int cast_shadow;
};

struct LightingUBO
{
    // camera
    vec3 camera_pos;
    float exposure;
    mat4 camera_view;
    mat4 inv_camera_view_proj;

    // lights
    SkyLight sky_light;
    DirectionalLight directional_light;
    // PointLight point_lights[MAX_POINT_LIGHT_NUM];
    // SpotLight spot_lights[MAX_SPOT_LIGHT_NUM];

    int has_sky_light;
    int has_directional_light;
    int point_light_num;
    int spot_light_num;

    // debug
    vec3 camera_dir;
    int shader_debug_option;
};

struct PbrMaterialInfo
{
    vec3 position;
    vec3 normal;
    vec4 base_color;
    vec4 emissive_color;
    float metallic;
    float roughness;
    float occlusion;
};

enum ShaderDebugOption : uint32_t
{
    Debug_Light = 0,
    Debug_Unlit,
    Debug_Wireframe,
    Debug_LightOnly,
    Debbug_Depth,
    Debug_Normal,
    Debug_BaseColor,
    Debug_EmissiveColor,
    Debug_Metallic,
    Debug_Roughness,
    Debug_Occulsion,
    Debug_Opacity
};

#endif