#ifndef __cplusplus
#define sdk_bool bool
// Standard SDK defines
#define SDK_BOOL  bool
#define SDK_VEC2  vec2
#define SDK_VEC3  vec3
#define SDK_VEC4  vec4
#define SDK_MAT4  mat4

#endif
struct Node
{
	float m;
	float d;
	float gx;
	float gy;
	float u;
	float v;
	float ax;
	float ay;
	int act;
};

struct Particle
{
	float x;
	float y;
	float u;
	float v;
	float pu;
	float pv;
	float d;

	int   cx;
	int   cy;

	float gu;
	float gv;
	float T00;
	float T01;
	float T11;

	float px[3];
	float py[3];
	float gx[3];
	float gy[3];
};
