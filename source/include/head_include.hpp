#pragma once

#include<iostream>
#include <memory>
#include <string>
#include <vector>


#include <glm/glm.hpp>

using glm::vec3;

using std::cout;
using std::endl;
using std::vector;


#define M_PI 3.1415926

inline float deg2rad(const float& deg) {
	return deg * M_PI / 180.0;
}
