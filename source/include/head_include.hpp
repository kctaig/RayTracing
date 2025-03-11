#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <random>
#include <glm/glm.hpp>

using glm::ivec2;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::normalize;
using glm::cross;
using glm::dot;

using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::shared_ptr;
using std::make_shared;

#define M_PI 3.1415926f
constexpr auto EPSILON = 1e-6f;

static float genRandomFloat(float rangeL = 0.0f, float rangeR = 1.0f) {
	static std::random_device rd;
	static std::default_random_engine gen(rd());
	static std::uniform_real_distribution<float> dist(rangeL, rangeR);
	return dist(gen);
}

static vec3 toWorld(const vec3& localDir, const vec3& n) {
	vec3 arbitrary = (std::abs(n.x) > 0.9f ? vec3(0, 1, 0) : vec3(1, 0, 0));
	vec3 tangent = normalize(cross(n, arbitrary));
	vec3 bitangent = cross(n, tangent);
	return localDir.x * tangent + localDir.y * bitangent + localDir.z * n;
}