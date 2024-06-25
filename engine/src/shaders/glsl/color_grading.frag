#version 450 core

layout(input_attachment_index = 0, set = 0, binding = 0) uniform highp subpassInput in_color;
layout(set = 0, binding = 1) uniform sampler2D color_grading_lut_texture;
layout(location = 0) out vec4 out_color;

void main()
{
    highp vec2 lut_tex_size = textureSize(color_grading_lut_texture, 0);
    highp vec4 color = subpassLoad(in_color).rgba;

    out_color = color;
}