#pragma once

#include"head_include.hpp"
#include <ray.hpp>

class Camera {
public:
	Camera() {};

	Camera(vec3 pos, vec3 up, vec3 right) :pos(pos), up(up), right(right){}

	Ray gen_primary_ray(float x,float y, float aspect_ratio) const ;

	vec3 get_pos()const { return pos; }
	float get_vfov()const { return vfov; }
	float get_focal_length()const { return focal_length; }

	void set_vfov(float deg) { vfov = deg; }
	void set_focal_length(float f) { focal_length = f; }

private:
	vec3 pos;
	vec3 up;
	vec3 right;
	float vfov = 90;
	float focal_length = 1;
};