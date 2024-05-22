#ifndef _STRUCT_H_
#define _STRUCT_H_

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

#endif