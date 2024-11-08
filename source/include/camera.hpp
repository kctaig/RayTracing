#pragma once

#include "head_include.hpp"
#include <ray.hpp>
#include "film.hpp"
#include "spin_lock.hpp"

class Camera
{
public:
	Camera() {};
	 
	Camera(vec3 p, vec3 up, vec3 v, float vfov = 45) :pos(p), up(up), viewPoint(v), vfov(vfov) {
		focalLen = 1 / tan(glm::radians(vfov) / 2);
	};
	void generateMatrix();

	Ray generateRay(const glm::ivec2 &pixelCoord, const glm::vec2 &offsets) const;

	Film *film;
	vec3 pos;
	vec3 up;
	vec3 viewPoint;
	float focalLen;
	float vfov;
	glm::mat4 camFromWorld;
	glm::mat4 clipFromCam;
};