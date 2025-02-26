#pragma once

#include "head_include.hpp"
#include "ray.hpp"
#include "film.hpp"

class Camera
{
public:
	Camera() {};

	Camera(vec3 e, vec3 l, vec3 u, float fovy = 90) : eye(e), lookat(l), up(u), fovy(fovy) {};

	Ray rayCasting(const glm::ivec2& pixelCoord) const;

	Film* filmPtr;
	vec3 eye;
	vec3 lookat;
	vec3 up;
	float fovy;
};