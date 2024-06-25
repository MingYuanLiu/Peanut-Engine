#ifndef _CONSTANTS_
#define _CONSTANTS_

#define PI 3.1415926535
#define SHADOW_CASCADE_NUM 4
#define DEBUG_SHADER_DEPTH_MULTIPLIER 0.02
#define DIELECTRIC_F0 0.04 // F0 of dielectric

#define STD_GAMMA 2.2
#define TONEMAP_EXPOSURE 4.5

#define TWO_PI 2 * PI
#define EPSILON 0.00001
#define NUM_SAMPLES_MonteCarlo 1024
#define INV_NUM_SAMPLES (1.0 / float(NUM_SAMPLES_MonteCarlo))

#endif