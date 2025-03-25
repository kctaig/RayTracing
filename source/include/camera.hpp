#pragma once

#include "head_include.hpp"
#include "ray.hpp"
#include "film.hpp"

class Camera
{
public:
	Camera() = default;

	Camera(vec3 e, vec3 l, vec3 u, float fovy) : eye(e), lookat(l), up(u), fovy(fovy) {};
	Ray rayCasting(const shared_ptr<Film>filmPtr, const glm::ivec2& pixelCoord) const;

private:
	vec3 eye{ 0 };
	vec3 lookat{ 0 };
	vec3 up{ 0 };
	float fovy{ 0 };
};