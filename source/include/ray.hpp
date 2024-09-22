#pragma once

#include "head_include.hpp"

class Ray {
public:
	Ray(vec3 pos, vec3 dir):origin(pos),dir(dir){}

	vec3 get_origin()const { return origin; }
	vec3 get_dir()const { return dir; }
	vec3 at(float t) const {return origin + t * dir;}

private:
	vec3 origin;
	vec3 dir;
};