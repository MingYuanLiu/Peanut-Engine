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

void main()
{
    
}

#endif