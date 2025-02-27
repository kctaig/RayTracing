#include "material.hpp"
#include "head_include.hpp"

float Material::pdf(const vec3 ray_in, const vec3 ray_out, const vec3 normal)
{
	return 0.5f / M_PI;
}

vec3 Material::brdf(const vec3 ray_in, const vec3 ray_out, const vec3 normal)
{
	return diffuse / M_PI;
}

vec3 Material::sampleDir(const vec3 ray_in, const vec3 normal)
{
	float x_1 = genRandomFloat();
	float x_2 = genRandomFloat();
	float z = std::fabs(1.0f - 2.0f * x_1);
	float r = std::sqrt(1.0f - z * z);
	float phi = 2 * M_PI * x_2;
	vec3 local_dir(r * std::cos(phi), r * std::sin(phi), z);
	vec3 tangent1, tangent2;
	// 选择一个与 N 不平行的向量，这里使用 (1, 0, 0)，假设 N 不与 (1, 0, 0) 重合
	vec3 arbitrary = fabs(normal.x > 0.99f) ? vec3(0, 1, 0) : vec3(1, 0, 0);
	tangent1 = normalize(cross(normal, arbitrary));
	tangent2 = normalize(cross(normal, tangent1));
	return local_dir.x * tangent1 + local_dir.y * tangent2 + local_dir.z * normal;
}