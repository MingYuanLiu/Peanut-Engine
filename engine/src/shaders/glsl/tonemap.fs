#version 450 core
#extension GL_GOOGLE_include_directive : enable

#include "hdr.h"

#define PureWhite 1.0 // todo: modify this param

layout(input_attachment_index = 0, binding = 0) uniform subpassInput scene_color;
layout(location=0) out vec4 out_color;

void main()
{
    vec3 color = subpassLoad(scene_color).rgb;
    // Reinhard tonemapping operator.
	// see: "Photographic Tone Reproduction for Digital Images", eq. 4
    outColor = Tonemap(color);
}