#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <random>
#include <glm/glm.hpp>

using glm::mat4;
using glm::vec3;
using std::cout;
using std::endl;
using std::vector;
using std::string;

#define M_PI 3.1415926

static float genRandomFloat(float rangeL = 0.0f, float rangeR = 1.0f) {
	static std::random_device rd;
	static std::default_random_engine gen(rd());
	static std::uniform_real_distribution<float> dist(rangeL, rangeR);
	return dist(gen);
}