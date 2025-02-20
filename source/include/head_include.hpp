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


static float genRandF(float range) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    // 定义均匀分布
    std::uniform_real_distribution<float> dist(-range, range);
    // 生成随机扰动
    return dist(gen);
}
