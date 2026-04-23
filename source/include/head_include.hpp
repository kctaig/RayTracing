#pragma once

#include <chrono>
#include <filesystem>
#include <glm.hpp>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <vector>

using glm::cross;
using glm::dot;
using glm::ivec2;
using glm::mat4;
using glm::normalize;
using glm::vec2;
using glm::vec3;
using glm::vec4;

using std::cout;
using std::endl;
using std::make_shared;
using std::shared_ptr;
using std::string;
using std::vector;

namespace fs = std::filesystem;

#define M_PI 3.1415926f
constexpr auto EPSILON = 1e-6f;

inline float genRandomFloat(const float rangeL = 0.0f, const float rangeR = 1.0f) {
    static std::random_device rd;
    static std::default_random_engine gen(rd());
    static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(gen) * (rangeR - rangeL) + rangeL;
}

inline vec3 toWorld(const vec3& localDir, const vec3& n) {
    vec3 arbitrary;
    if (std::abs(n.x) > std::abs(n.y) && std::abs(n.x) > std::abs(n.z)) {
        arbitrary = vec3(0, 1, 0);
    } else if (std::abs(n.y) > std::abs(n.z)) {
        arbitrary = vec3(0, 0, 1);
    } else {
        arbitrary = vec3(1, 0, 0);
    }
    const vec3 tangent = normalize(cross(n, arbitrary));
    const vec3 bitangent = normalize(cross(n, tangent));
    const vec3 worldDir = localDir.x * tangent + localDir.y * bitangent + localDir.z * n;
    return normalize(worldDir);
}

inline float powerHeuristic(const float pdf1, const float pdf2) {
    const float f1 = pdf1 * pdf1;
    const float f2 = pdf2 * pdf2;
    return (f1 + f2) == 0.f ? 0.f : f1 / (f1 + f2);
}

inline bool isNAN(const vec3& v) { return v.x != v.x || v.y != v.y || v.z != v.z; }

inline bool fileExists(const std::string& sceneDir, const std::string& fileName) {
    return fs::exists(fs::path(sceneDir) / fileName / (fileName + ".xml"));
}