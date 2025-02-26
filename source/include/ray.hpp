#pragma once

#include "head_include.hpp"

class PayLoad
{
public:
	PayLoad() : matId(-1), hitPos(vec3(0)), normal(vec3(0)), uv(vec2(0)), t(FLT_MAX) {}

	int matId;
	vec3 hitPos;
	vec3 normal;
	vec2 uv;
	float t;
};

class Ray
{
public:
	Ray(vec3 pos, vec3 dir) : origin(pos), dir(dir) {}

	vec3 getOrigin() const { return origin; }
	vec3 getDir() const { return dir; }
	vec3 at(float t) const { return origin + dir * t; }

private:
	vec3 origin;
	vec3 dir;
};