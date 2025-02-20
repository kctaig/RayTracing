#include "material.hpp"
#include "head_include.hpp"

float Material::pdf(const vec3 ray_in, const vec3 ray_out, const vec3 normal)
{
	if (dot(ray_out, normal) > 0.0f)
		return 1.0f / (2.0f * M_PI);
	else return 0.0f;
}

vec3 Material::brdf(const vec3 ray_in, const vec3 ray_out, const vec3 normal)
{
	float cos_theta = dot(ray_out, normal);
	if (cos_theta > 0.0f)
		return diffuse * static_cast<float>(1.0f) / static_cast<float>(M_PI);
	else return vec3(0.f);
}

vec3 Material::sampleDir(const vec3 ray_in, const vec3 normal)
{
	return vec3();
}