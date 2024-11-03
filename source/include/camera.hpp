#pragma once

#include "head_include.hpp"
#include <ray.hpp>
#include "film.hpp"

class Camera
{
public:
	Camera() {};
	 
	Camera(vec3 p, vec3 u, vec3 v, float vfov = 90) :pos(p), up(u), v(v), vfov(vfov) {};

	void generateMatrix();

	Ray generateRay(const glm::ivec2 &pixelCoord, const glm::vec2 &offsets) const;

	Film *film;
	vec3 pos;
	vec3 up;
	vec3 v;
	float vfov;
	glm::mat4 camFromWorld;
	glm::mat4 clipFromCam;
};