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
using glm::mat4;
using glm::normalize;
using glm::cross;
using glm::dot;

using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::shared_ptr;

#define M_PI 3.1415926f
#define EPLISON 0.0001f

static float genRandomFloat(float rangeL = 0.0f, float rangeR = 1.0f) {
	static std::random_device rd;
	static std::default_random_engine gen(rd());
	static std::uniform_real_distribution<float> dist(rangeL, rangeR);
	return dist(gen);
}