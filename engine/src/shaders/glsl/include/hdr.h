#ifndef _HDR_H_
#define _HDR_H_

#include "constants.h"

vec3 Uncharted2Tonemap(vec3 color)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	float W = 11.2;
	return ((color*(A*color+C*B)+D*E)/(color*(A*color+B)+D*F))-E/F;
}

vec3 Tonemap(vec3 color)
{
	vec3 uncharted_color = Uncharted2Tonemap(color * TONEMAP_EXPOSURE);
	uncharted_color = uncharted_color * (1.0f / Uncharted2Tonemap(vec3(11.2f)));	
	return pow(uncharted_color, vec3(1.0f / STD_GAMMA));
}

vec3 SRGBToLinear(vec3 srgb)
{
    return pow(srgb, vec3(STD_GAMMA));
}

vec3 LinearToSRGB(vec3 linear)
{
    return pow(linear, vec3(1.0 / STD_GAMMA));
}
#endif