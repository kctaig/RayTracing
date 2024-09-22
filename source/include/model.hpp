#pragma once

#include "head_include.hpp"

struct Vertex {
    vec3 pos;
    vec3 normal;
    vec3 color;
};

struct Mesh {
    vector<size_t> indices;
};

struct Model {
    vector<Vertex> vertices; // 模型顶点
    vector<Mesh>meshes;    // 模型的面片
};
