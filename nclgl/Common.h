/******************************************************************************
Author:Rich Davison
Description: Some random variables and functions, for lack of a better place 
to put them.

-_-_-_-_-_-_-_,------,   
_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
-_-_-_-_-_-_-~|__( ^ .^) /
_-_-_-_-_-_-_-""  ""   

*//////////////////////////////////////////////////////////////////////////////

#pragma once

//#include "../../GLEW/include/GL/glew.h"
#include "GL/glew.h"

#define WEEK_2_CODE
#define WEEK_3_CODE

//It's pi(ish)...
static const float		PI = 3.14159265358979323846f;	

//It's pi...divided by 360.0f!
static const float		PI_OVER_360 = PI / 360.0f;

//Radians to degrees
static inline double RadToDeg(const double deg)	{
	return deg * 180.0 / PI;
};

//Arent these the wrong way around? You pass a radian value to be converted to
//degrees!
//Degrees to radians
static inline double DegToRad(const double rad)	{
	return rad * PI / 180.0;
};

static inline unsigned int GetLastTextureGPUMem(){
	int w,h;

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

	return 4 * w * h;

}

//Cheap random number generator, will generate
//numbers between 0.0 and 1.0 to 2 DP
#define RAND() ((rand()%101)/100.0f)

//I blame Microsoft...
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))

#define SHADERDIR	"../Shaders/"
#define MESHDIR		"../Meshes/"
#define TEXTUREDIR  "../Textures/"
#define SOUNDSDIR	"../Sounds/"