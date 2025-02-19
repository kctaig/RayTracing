#pragma once

#include "head_include.hpp"

class PayLoad {
public:
	bool ishit = false;
	int mat_id;
	vec3 hitPos;
	vec3 normal;
	glm::vec2 uv;
};

class Ray {
public:
	Ray(vec3 pos, vec3 dir) :origin(pos), dir(dir) {}

	vec3 get_origin()const { return origin; }
	vec3 get_dir()const { return dir; }
	vec3 at(float t) const { return origin + dir * t; }

private:
	vec3 origin;
	vec3 dir;
};