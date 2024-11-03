#pragma once

#include "head_include.hpp"

class Vertex {

public:
    vec3 pos;
    vec3 normal;
    vec3 color;
};

class Mesh {

public:
    vector<size_t> indices;
};

class Model {

public:
    Model() {}
    vector<Vertex> vertices; // 模型顶点
    vector<Mesh>meshes;    // 模型的面片
};
